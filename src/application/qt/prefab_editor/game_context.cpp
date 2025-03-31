#include <assets/asset_manager.hpp>
#include <common/logging.hpp>
#include <graphics/graphics.hpp>
#include <graphics/graphics_fwd.hpp>
#include <project/component_interface/component.hpp>
#include <project/components/camera.hpp>
#include <project/components/mesh_filter.hpp>
#include <project/components/mesh_renderer.hpp>
#include <project/game_object.hpp>
#include <project/project_manager.hpp>
#include <project/scene.hpp>

#include "game_context.hpp"

inline logger log() { return get_logger("game_context"); }

void game_context::set_viewport_size(glm::uvec2 size)
{
    auto cam = components::camera::get_active();
    if (!cam)
    {
        return;
    }
    cam->set_render_size(size);
}

void game_context::initialize()
{
    log()->info("Initializing game context");

    project_manager::initialize();
    scene::create();

    log()->info("Game context initialized successfully");
}

void game_context::load_assets()
{
    assets::asset_manager::initialize("resources");
    assets::asset_manager::scan_project_directory();
    assets::asset_manager::setup_project_directory_watch();
}

void game_context::render()
{
    auto cam = components::camera::get_active();
    if (!cam)
    {
        return;
    }
    cam->render();
}

void game_context::init()
{
    scene::get_active_scene()->visit_root_objects([](auto obj)
    { obj->init(); });
}

void game_context::update()
{
    scene::get_active_scene()->visit_root_objects([](auto obj)
    { obj->update(); });
}

void game_context::deinit()
{
    scene::get_active_scene()->visit_root_objects([](auto obj)
    { obj->deinit(); });
}

void game_context::create_empty_game_object()
{
    auto new_object = game_object::create();
    new_object->set_name("New Game Object");

    log()->info("New game object {} ({}) was created",
                new_object->get_name(),
                new_object->id().id);
    new_object->init();
    scene::get_active_scene()->add_root_object(new_object);
}

void game_context::create_camera()
{
    auto obj = game_object::create();
    obj->set_name("New Game Object");

    log()->info(
        "New game object {} ({}) was created", obj->get_name(), obj->id().id);

    auto& cam = obj->add<components::camera>();
    cam.set_background_color({ 0.1f, 0.1f, 0.1f, 1.0f });
    cam.set_active();
    obj->init();
    scene::get_active_scene()->add_root_object(obj);
}

void game_context::create_cube()
{
    auto obj = game_object::create();
    obj->set_name("New Game Object");

    log()->info(
        "New game object {} ({}) was created", obj->get_name(), obj->id().id);

    obj->add<components::mesh_filter>().set_mesh(
        assets::asset_manager::get<graphics::mesh>("meshes.cube.fbx"));
    obj->add<components::mesh_renderer>().add_material(
        assets::asset_manager::get<graphics::material>(
            "standard.standard.mat"));
    obj->init();
    scene::get_active_scene()->add_root_object(obj);
}
