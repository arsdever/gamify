/* clang-format off */
#include <GLFW/glfw3.h>
/* clang-format on */

#include "assets/asset_manager.hpp"
#include "common/filesystem.hpp"
#include "common/logging.hpp"
#include "core/viewport.hpp"
#include "core/window.hpp"
#include "graphics/graphics.hpp"
#include "graphics/material.hpp"
#include "graphics/texture.hpp"
#include "project/components/camera.hpp"
#include "project/components/light.hpp"
#include "project/components/mesh_filter.hpp"
#include "project/components/mesh_renderer.hpp"
#include "project/components/transform.hpp"
#include "project/game_object.hpp"
#include "project/project_manager.hpp"
#include "project/scene.hpp"

namespace
{
logger log() { return get_logger("window example"); }
} // namespace

void init_scene();

int main(int argc, char** argv)
{
    project_manager::initialize();
    glfwInit();
    std::vector<std::shared_ptr<core::window>> windows;
    auto exp_window = std::make_shared<core::window>();

    std::shared_ptr<game_object> main_camera_object = nullptr;
    std::shared_ptr<game_object> camera2_object = nullptr;
    std::shared_ptr<scene> current_scene = nullptr;

    exp_window->on_user_initialize +=
        [ &current_scene, &main_camera_object, &camera2_object ](
            std::shared_ptr<core::window> wnd)
    {
        // configure gl debug output
        graphics::initialize(glfwGetProcAddress);
        assets::asset_manager::initialize(
            (common::filesystem::path::current_dir() / "resources")
                .full_path());
        main_camera_object = game_object::create();
        camera2_object = game_object::create();
        main_camera_object->add<components::camera>();
        camera2_object->add<components::camera>();
        current_scene = scene::create();
        main_camera_object->get<components::camera>().set_active();
        init_scene();
        auto vp = std::make_shared<core::viewport>();
        vp->set_size(glm::uvec2(wnd->get_size().x / 2, wnd->get_size().y));
        wnd->add_viewport(vp);
        vp = std::make_shared<core::viewport>();
        vp->set_size(glm::uvec2(wnd->get_size().x / 2, wnd->get_size().y));
        vp->set_position(glm::uvec2(wnd->get_size().x / 2, 0));
        wnd->add_viewport(vp);
    };

    exp_window->set_title("Hello experimental window");
    exp_window->init();

    exp_window->set_can_grab(true);

    exp_window->get_events()->close += [ &windows ](auto ce)
    { std::erase(windows, ce.get_sender()->shared_from_this()); };

    exp_window->get_events()->resize += [](auto re)
    {
        auto wnd = re.get_sender();
        if (auto vp = wnd->get_viewports()[ 0 ])
        {
            vp->set_size(wnd->get_size());
        }
    };

    int frame_counter = 0;
    exp_window->get_events()->render += [ &frame_counter ](auto re)
    {
        if (frame_counter % 30 == 0)
        {
            graphics::clear(glm::dvec4 { 0, 0, 0, 1 });
            int i = 0;
            for (auto vp : re.get_sender()->get_viewports())
            {
                vp->take_screenshot(std::format(
                    "screenshots/screenshot_{}_{}.png", i, frame_counter));
                ++i;
            }
        }
        ++frame_counter;
    };

    windows.push_back(exp_window);

    while (!windows.empty())
    {
        for (int i = 0; i < windows.size(); ++i)
        {
            auto wnd = windows[ i ];
            wnd->update();
        }
    }

    assets::asset_manager::shutdown();
    project_manager::shutdown();
    return 0;
}

void init_scene()
{
    auto basic_mat =
        assets::asset_manager::get<graphics::material>("standard.standard.mat");
    auto txt =
        assets::asset_manager::get<graphics::texture>("images.diffuse.png");
    auto norm_txt =
        assets::asset_manager::get<graphics::texture>("images.brick.png");
    basic_mat->set_property_value("albedo_texture_strength", 0.0f);
    basic_mat->set_property_value("albedo_color", 1.0f, 0.8f, 0.2f);
    basic_mat->set_property_value("normal_texture_strength", 0.0f);

    components::camera::all()[ 0 ]->get_transform()->set_position({ 0, 0, 3 });
    components::camera::all()[ 0 ]->get_transform()->set_rotation(
        glm::quatLookAt(glm::vec3 { 0.0f, 0.0f, 1.0f },
                        glm::vec3 {
                            0.0f,
                            1.0f,
                            0.0f,
                        }));
    components::camera::all()[ 0 ]->set_orthogonal(false);
    components::camera::all()[ 0 ]->set_background_color(
        glm::dvec4(0.3, 0.6, 0.7, 1.0));

    components::camera::all()[ 1 ]->get_transform()->set_position({ 3, 0, 0 });
    components::camera::all()[ 1 ]->get_transform()->set_rotation(
        glm::quatLookAt(glm::vec3 { 1.0f, 0.0f, 0.0f },
                        glm::vec3 {
                            0.0f,
                            1.0f,
                            0.0f,
                        }));
    components::camera::all()[ 1 ]->set_orthogonal(false);
    components::camera::all()[ 1 ]->set_fov(90);
    components::camera::all()[ 1 ]->set_background_color(
        glm::dvec4(0.7, 0.6, 0.1, 1.0));

    auto obj = game_object::create();
    auto m =
        assets::asset_manager::get<graphics::mesh>("meshes.susane_head.fbx");
    obj->add<components::mesh_filter>().set_mesh(m);
    obj->add<components::mesh_renderer>().set_material(0, basic_mat);
    obj->set_name("susane");
    scene::get_active_scene()->add_root_object(obj);

    auto lo = game_object::create();
    auto& l = lo->add<components::light>();
    l.set_color(glm::dvec4(1.0));
    l.set_intensity(5.0);
}
