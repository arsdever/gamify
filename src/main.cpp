#include <GLFW/glfw3.h>
#include <prof/profiler.hpp>

#include "core/asset_manager.hpp"
// #include "components/box_collider_component.hpp"
// #include "components/camera_component.hpp"
// #include "components/fps_show_component.hpp"
// #include "components/light_component.hpp"
// #include "components/mesh_component.hpp"
// #include "components/mesh_renderer_component.hpp"
// #include "components/plane_collider_component.hpp"
// #include "components/ray_visualize_component.hpp"
// #include "components/text_component.hpp"
// #include "components/text_renderer_component.hpp"
// #include "components/walking_component.hpp"
#include "experimental/input_system.hpp"
#include "experimental/viewport.hpp"
#include "experimental/window.hpp"
#include "experimental/window_events.hpp"
#include "feature_flags.hpp"
#include "file.hpp"
#include "font.hpp"
#include "game_clock.hpp"
#include "graphics/gpu.hpp"
#include "image.hpp"
#include "input_system.hpp"
#include "logging.hpp"
#include "material.hpp"
#include "project/components/camera.hpp"
#include "project/components/component_registry.hpp"
#include "project/components/light.hpp"
#include "project/components/mesh_filter.hpp"
#include "project/components/mesh_renderer.hpp"
#include "project/components/transform.hpp"
#include "project/game_object.hpp"
#include "scene.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "texture_viewer.hpp"
#include "thread.hpp"
#include "tools/mesh_viewer/mesh_viewer.hpp"

using namespace experimental;

namespace
{
static logger log() { return get_logger("main"); }
unsigned last_fps;
font ttf;
std::shared_ptr<scene> s;
std::unordered_set<int> pressed_keys;
std::shared_ptr<game_object> main_camera_object;
game_object* _fps_text_object;
texture* txt;
texture* norm_txt;
// ray_visualize_component* cast_ray;
std::vector<std::shared_ptr<experimental::window>> windows;
std::string console_string;
int mesh_preview_texture_index { 0 };

// physics_engine p;
} // namespace

void load_internal_resources();
void initScene();
void initMainWindow();
void initProfilerView();
// void initViewports();
void setupMouseEvents();

void on_error(int error_code, const char* description)
{
    log()->error("Error {}: {}", error_code, description);
}

static std::atomic_int counter = 0;

int main(int argc, char** argv)
{
    // register components
    component_registry::register_components();
    // components::box_collider::register_component();
    // components::plane_collider::register_component();
    // components::walking::register_component();
    // components::light::register_component();
    // components::text_renderer::register_component();
    // components::text::register_component();
    // components::camera::register_component();
    // components::fps_show::register_component();
    // components::ray_visualize::register_component();

    configure_levels(argc, argv);
    game_clock* clock = game_clock::init();
    glfwInit();
    glfwSetErrorCallback(on_error);
    adjust_timeout_accuracy_guard guard;

    game_object* selected_object = nullptr;

    initMainWindow();
    // initProfilerView();
    // initViewports();
    setupMouseEvents();
    initScene();

    auto mv = std::make_shared<mesh_viewer>();
    mv->init();
    mv->set_mesh(asset_manager::default_asset_manager()->meshes()[ 4 ]);
    windows.push_back(mv);

    log()->info("Using graphics card {} - {}",
                graphics::gpu::get_vendor(),
                graphics::gpu::get_device());

    // TODO: may not be the best place for object initialization
    // Probably should be done in some sort of scene loading procedure
    if (scene::get_active_scene())
    {
        scene::get_active_scene()->visit_root_objects([](auto obj)
        { obj->init(); });
    }

    file mat_file("resources/standard/standard.mat");
    mat_file.changed_externally +=
        [](file::event_type et) { log()->info("material file changed"); };
    mat_file.watch();

    // start a physics thread
    // TODO: these should move into physics engine class
    std::atomic_bool program_exits = false;
    // TODO: make this variable
    static constexpr std::atomic<double> physics_fps = 500.0;
    std::thread thd { [ &program_exits, clock ]
    {
        double physics_frame_time_hint = 1.0 / physics_fps;
        while (!program_exits)
        {
            auto _physics_frame_start = std::chrono::steady_clock::now();
            // do stuff

            clock->physics_frame();
            std::this_thread::sleep_until(
                _physics_frame_start +
                std::chrono::duration<double>(physics_frame_time_hint));
        }
    } };
    set_thread_name(thd, "physics_thread");
    set_thread_priority(thd, 15);
    auto camera = components::camera::get_active();
    // camera->set_background(glm::dvec4 { 1.0, 0.0, 0.0, 0.0 });
    // texture* txt = new texture;
    // main_camera->set_render_texture(txt);

    int trigger_show = -1;
    bool console_mode = false;

    auto wh = file::watch("./",
                          [](auto path, auto change)
    { log()->info("Path {} changed: {}", path, static_cast<int>(change)); });

    experimental::input_system::on_keypress +=
        [ &console_mode, &trigger_show, mv ](int keycode)
    {
        if (keycode == GLFW_KEY_GRAVE_ACCENT)
        {
            console_mode = !console_mode;
            if (console_mode)
            {
                log()->info("Entering console mode:");
            }
            else
            {
                log()->info("Leaving console mode");
            }
        }

        if (!console_mode)
            return;

        if (keycode == GLFW_KEY_ENTER)
        {
            if (console_string.empty())
                return;

            std::transform(console_string.begin(),
                           console_string.end(),
                           console_string.begin(),
                           ::tolower);

            if (console_string.starts_with("show"))
            {
                console_string = console_string.substr(5);
                if (console_string.starts_with("texture"))
                {
                    console_string = console_string.substr(7);
                    int num = std::stoi(console_string);

                    if (num >= 0 && num < texture::_textures.size())
                    {
                        trigger_show = num;
                    }
                }
                else if (console_string.starts_with("mesh"))
                {
                    console_string = console_string.substr(4);
                    int num = std::stoi(console_string);
                    mesh* m { nullptr };
                    if (num >= 0 && num < asset_manager::default_asset_manager()
                                              ->meshes()
                                              .size())
                    {
                        m = asset_manager::default_asset_manager()
                                ->meshes()[ num ];
                    }
                    mv->set_mesh(m);
                }
            }
            if (console_string.starts_with("clone"))
            {
                console_string = console_string.substr(6);
                int num = std::stoi(console_string);
                if (num >= 0 && num < texture::_textures.size())
                {
                    (new texture)->clone(texture::_textures[ num ]);
                }
            }
            if (console_string.starts_with("print textures"))
            {
                for (int i = 0; i < texture::_textures.size(); ++i)
                {
                    log()->info("Texture {}: id {}",
                                i,
                                texture::_textures[ i ]->native_id());
                }
            }

            console_string = "";
        }

        if (keycode < 255 && (std::isalnum(keycode) || std::isspace(keycode)))
        {
            console_string += keycode;
        }

        if (keycode == GLFW_KEY_BACKSPACE)
        {
            if (!console_string.empty())
            {
                console_string.pop_back();
            }
        }

        log()->info("Console: {}", console_string);
    };

    while (!windows.empty())
    {
        scene::get_active_scene()->visit_root_objects([](auto obj)
        { obj->update(); });

        for (int i = 0; i < windows.size(); ++i)
        {
            auto p = prof::profile_frame(__FUNCTION__);
            auto window = windows[ i ];
            window->update();
        }
        clock->frame();

        if (trigger_show >= 0)
        {
            texture* txt = texture::_textures[ trigger_show ];
            log()->info("Showing texture at {} with id {}",
                        trigger_show,
                        txt->native_id());
            texture_viewer::show_preview(txt);
            trigger_show = -1;
        }
    }

    std::stringstream ss;
    ss << std::this_thread::get_id();
    prof::apply_data(ss.str(),
                     [](const prof::data_sample& data) -> bool
    {
        log()->info("Profiling frame:\n\tfunction name: {}\n\tdepth: "
                    "{}\n\tduration: {}",
                    data.name(),
                    data.depth(),
                    std::chrono::duration_cast<std::chrono::duration<double>>(
                        data.end() - data.start())
                        .count());
        return true;
    });
    std::cout << std::flush;
    program_exits = true;
    glfwTerminate();
    thd.join();
    return 0;
}

void initMainWindow()
{
    windows.push_back(std::make_shared<experimental::window>());
    auto& wnd = windows.back();
    wnd->on_user_initialize += [](auto) { load_internal_resources(); };
    wnd->init();
    wnd->resize(800, 800);
    wnd->get_events()->close += [](auto ce)
    { std::erase(windows, ce.get_sender()->shared_from_this()); };
    std::shared_ptr<viewport> vp = std::make_shared<viewport>();
    vp->initialize();
    wnd->add_viewport(vp);
    wnd->get_events()->resize +=
        [ vp ](auto re) { vp->set_size(re.get_new_size()); };
    wnd->get_events()->render += [ vp ](auto re)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        vp->render();
    };
    windows.back()->get_events()->resize += [ vp ](auto re)
    {
        vp->set_size(
            { re.get_sender()->get_width(), re.get_sender()->get_height() });
    };

    wnd->set_can_grab(true);
    vp->set_size({ wnd->get_width(), wnd->get_height() });
}

// void initViewports()
// {
//     windows.push_back(std::make_shared<experimental::window>());
//     auto wnd = windows.back();
//     wnd->init();
//     wnd->resize(400, 1200);
//     windows[ 0 ]->set_position(windows[ 1 ]->get_position().x +
//                                    windows[ 1 ]->get_width(),
//                                windows[ 1 ]->get_position().y);

//     wnd->get_events()->close += [](auto ce)
//     { std::erase(windows, ce.get_sender()->shared_from_this()); };

//     for (int i = 0; i < _view_cameras.size(); ++i)
//     {
//         _view_cameras[ i ] = std::make_shared<camera>();
//         auto cam = _view_cameras[ i ];
//         auto vp = std::make_shared<experimental::viewport>();
//         vp->initialize();
//         vp->set_camera(cam);
//         cam->set_ortho(true);
//         wnd->add_viewport(vp);
//         wnd->get_events()->mouse_scroll += [ cam, vp ](auto we)
//         {
//             auto pos = we.get_local_position();
//             pos.y = we.get_sender()->get_height() - pos.y;
//             if (!rect_contains(vp->get_position(),
//                                vp->get_position() + vp->get_size(),
//                                pos))
//             {
//                 return;
//             }

//             cam->get_transform().set_position(
//                 cam->get_transform().get_position() *
//                 std::pow<float>(1.2, we.get_delta().y));
//         };
//         windows.back()->get_events()->resize += [ vp, i ](auto re)
//         {
//             vp->set_size({ re.get_new_size().x, re.get_new_size().y / 3.0
//             }); vp->set_position({ 0, re.get_new_size().y / 3.0 * i });
//         };

//         vp->set_size({ wnd->get_width(), wnd->get_height() / 3.0 });
//         vp->set_position({ 0, wnd->get_height() / 3.0 * i });
//     }

//     windows.back()->get_events()->render += [](auto re)
//     {
//         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//         for (auto vp : re.get_sender()->get_viewports())
//         {
//             vp->render();
//         }
//     };
//     _view_cameras[ 0 ]->get_transform().set_position({ 100, 0, 0 });
//     _view_cameras[ 0 ]->get_transform().set_rotation(glm::quatLookAt(
//         glm::vec3 { -1.0f, 0.0f, 0.0f }, glm::vec3 { 0.0f, 1.0f, 0.0f
//         }));
//     _view_cameras[ 0 ]->set_background(glm::vec3 { 0.1f, 0.0f, 0.0f });

//     _view_cameras[ 1 ]->get_transform().set_position({ 0, 100, 0 });
//     _view_cameras[ 1 ]->get_transform().set_rotation(glm::quatLookAt(
//         glm::vec3 { 0.0f, -1.0f, 0.0f }, glm::vec3 { 0.0f, 0.0f, 1.0f
//         }));
//     _view_cameras[ 1 ]->set_background(glm::vec3 { 0.0f, 0.1f, 0.0f });

//     _view_cameras[ 2 ]->get_transform().set_position({ 0, 0, 100 });
//     _view_cameras[ 2 ]->get_transform().set_rotation(glm::quatLookAt(
//         glm::vec3 { 0.0f, 0.0f, -1.0f }, glm::vec3 { 0.0f, 1.0f, 0.0f
//         }));
//     _view_cameras[ 2 ]->set_background(glm::vec3 { 0.0f, 0.0f, 0.1f });
// }

void setupMouseEvents()
{
    experimental::window::get_main_window()->get_events()->mouse_move +=
        [](auto me)
    {
        if (me.get_sender()->get_has_grab())
        {
            main_camera_object->get_transform().set_rotation(
                glm::quat(glm::radians(glm::vec3(-me.get_local_position().y,
                                                 -me.get_local_position().x,
                                                 0) *
                                       .1f)));
        }
        else
        {
            auto cam = components::camera::get_active();
            // draw ray casted from camera
            auto pos = cam->get_transform().get_position();
            auto rot = cam->get_transform().get_rotation();
            glm::vec3 point =
                glm::unProject(glm::vec3 { me.get_local_position().x,
                                           me.get_sender()->get_height() -
                                               me.get_local_position().y,
                                           0 },
                               cam->view_matrix(),
                               cam->projection_matrix(),
                               glm::vec4 { 0,
                                           0,
                                           me.get_sender()->get_width(),
                                           me.get_sender()->get_height() });
            // cast_ray->set_ray(pos, glm::normalize(point - pos));

            // auto hit = p.raycast(pos, glm::normalize(point - pos));

            // if (hit.has_value())
            // {
            //     log()->info("hit collider");
            // }
        }
    };
}

void initScene()
{
    s = scene::create();
    feature_flags::set_flag(feature_flags::flag_name::load_fbx_as_scene, false);
    auto* am = asset_manager::default_asset_manager();
    material* basic_mat = am->get_material("standard");
    txt = new texture();
    image* img = am->get_image("albedo");
    *txt = std::move(texture::from_image(img));
    norm_txt = new texture();
    img = am->get_image("brick");
    *norm_txt = std::move(texture::from_image(img));
    auto roughness_txt = new texture;
    img = am->get_image("roughness");
    *roughness_txt = std::move(texture::from_image(img));
    auto metallic_txt = new texture;
    img = am->get_image("metallic");
    *metallic_txt = std::move(texture::from_image(img));
    basic_mat->set_property_value("u_albedo_texture", txt);
    basic_mat->set_property_value("u_albedo_texture_strength", 1.0f);
    basic_mat->set_property_value("u_normal_texture", norm_txt);
    basic_mat->set_property_value("u_normal_texture_strength", 1.0f);
    basic_mat->set_property_value("u_roughness", 1.0f);
    basic_mat->set_property_value("u_roughness_texture", roughness_txt);
    basic_mat->set_property_value("u_roughness_texture_strength", 1.0f);
    basic_mat->set_property_value("u_metallic", 1.0f);
    basic_mat->set_property_value("u_metallic_texture", metallic_txt);
    basic_mat->set_property_value("u_metallic_texture_strength", 1.0f);
    basic_mat->set_property_value("u_ao", 0.1f);

    auto object = game_object::create();
    object->add<components::mesh_filter>();

    object->get<components::mesh_filter>().set_mesh(
        am->get_mesh("Shader Ball JL 01_mesh"));
    object->add<components::mesh_renderer>().set_material(basic_mat);
    object->set_name("susane");
    s->add_root_object(object);
    // bc->set_position(glm::vec3(0, 0, 0));
    // bc->set_scale(glm::vec3(2, 1, 1));
    // bc->set_rotation(glm::quat(glm::ballRand(1.0f)));
    // bc->set_rotation(glm::quat(glm::radians(glm::vec3 { 0, 30, 0 })));

    ttf.load("font.ttf", 16);
    // game_object* collision_text_object = new game_object;
    // auto* ct = collision_text_object->create_component<text_component>();
    // collision_text_object->create_component<text_renderer_component>();
    // collision_text_object->get_component<text_renderer_component>()->set_font(
    //     &ttf);
    // collision_text_object->get_component<text_renderer_component>()
    //     ->set_material(am->get_material("text"));
    // s.add_object(collision_text_object);
    // bc->_text = ct;
    // collision_text_object->get_transform().set_scale({ 0.005f,
    // 0.005f, 1.0f
    // });

    // game_object* ray = new game_object;
    // cast_ray = ray->create_component<ray_visualize_component>();
    // ray->set_name("cast_ray");
    // s.add_object(ray);
    // cast_ray->set_ray({ 0, 0, 0 }, { 0, 0, 1 });

    main_camera_object = game_object::create();
    // main_camera_object->create_component<mesh_component>()->set_mesh(
    //     am->get_mesh("camera"));
    // main_camera_object->create_component<mesh_renderer_component>()
    //     ->set_material(am->get_material("standard"));
    auto& main_camera = main_camera_object->add<components::camera>();
    main_camera_object->set_name("main_camera");
    s->add_root_object(main_camera_object);

    // _fps_text_object = new game_object;
    // _fps_text_object->create_component<text_component>();
    // _fps_text_object->create_component<text_renderer_component>();
    // _fps_text_object->create_component<fps_show_component>();
    // _fps_text_object->get_component<text_renderer_component>()->set_font(&ttf);
    // _fps_text_object->get_component<text_renderer_component>()->set_material(
    //     am->get_material("text"));
    // am->get_material("text")->set_property_value(
    //     "u_text_color", 1.0f, 1.0f, 1.0f);
    // _fps_text_object->get_transform().set_position({ 0.5f, 2.0f, 0.0f });
    // _fps_text_object->get_transform().set_scale({ 0.01f, 0.01f, 1.0f });
    // _fps_text_object->set_name("fps_text");
    // s.add_object(_fps_text_object);

    main_camera_object->get_transform().set_position({ 0, 0, 3 });
    main_camera.get_transform().set_rotation(
        glm::quatLookAt(glm::vec3 { 0.0f, 0.0f, -1.0f },
                        glm::vec3 {
                            0.0f,
                            1.0f,
                            0.0f,
                        }));
    // main_camera_object->create_component<walking_component>();
    main_camera.set_orthogonal(false);
    main_camera.set_active();

    auto light_obj_1 = game_object::create();
    auto& l = light_obj_1->add<components::light>();
    l.set_color(glm::dvec4(1.0));
    l.set_intensity(10.0);
    // object = new game_object;
    // object->create_component<light_component>()->set_light(l);
    light_obj_1->get_transform().set_position(glm::vec3(0.0f, 1.5f, 0.0f));
    // s.add_object(object);
    light_obj_1->set_name("light_1");
    s->add_root_object(light_obj_1);

    auto light_obj_2 = game_object::create();
    auto& l2 = light_obj_2->add<components::light>();
    l2.set_color(glm::dvec4(1.0));
    l2.set_intensity(10.0);
    // object = new game_object;
    // object->create_component<light_component>()->set_light(l);
    light_obj_2->get_transform().set_position(glm::vec3(5.0f, 0.0f, 0.0f));
    light_obj_2->set_name("light_1");
    s->add_root_object(light_obj_2);
}

void load_internal_resources()
{
    auto* am = asset_manager::default_asset_manager();
    am->load_asset("resources/internal/camera_background.shader");

    am->load_asset("resources/meshes/cube.fbx");
    am->load_asset("resources/meshes/sphere.fbx");
    am->load_asset("resources/meshes/env_sphere.fbx");
    am->load_asset("resources/meshes/susane_head.fbx");
    am->load_asset("resources/meshes/shader_ball.fbx");
    am->load_asset("resources/meshes/camera.fbx");
    am->load_asset("resources/standard/text.mat");
    am->load_asset("resources/standard/skybox.mat");
    am->load_asset("resources/standard/surface.mat");
    am->load_asset("resources/standard/standard.mat");
    am->load_asset("resources/standard/mesh_viewer.mat");
    am->load_asset("resources/images/sample.png");
    am->load_asset("resources/images/brick.png");
    am->load_asset("resources/images/diffuse.png");
    am->load_asset("resources/images/albedo.jpg");
    am->load_asset("resources/images/metallic.jpg");
    am->load_asset("resources/images/roughness.jpg");
    am->load_asset("resources/images/env.jpg");
}

void initProfilerView()
{
    windows.push_back(std::make_shared<experimental::window>());
    auto& wnd = windows.back();
    wnd->init();
    wnd->resize(800, 200);
    wnd->get_events()->close += [](auto ce)
    { std::erase(windows, ce.get_sender()->shared_from_this()); };
    wnd->get_events()->render += [](auto re)
    {
        // prof::draw_data dd;
        // dd.height = re.get_sender()->get_height();
        // dd.width = re.get_sender()->get_width();
        // dd.zoom_x = .01f;
        // dd.zoom_y = 1.0f;
        // std::stringstream ss;
        // ss << std::this_thread::get_id();
        // prof::draw_overall_data(ss.str(), dd);
    };
    wnd->set_can_grab(false);
}
