#include <GLFW/glfw3.h>
#include <prof/profiler.hpp>

#include "graphics/graphics_fwd.hpp"

#include "application.hpp"

#include "application/application_commands.hpp"
#include "assets/asset_manager.hpp"
#include "common/filesystem.hpp"
#include "common/logging.hpp"
#include "common/main_thread_dispatcher.hpp"
#include "core/command_dispatcher.hpp"
#include "core/game_clock.hpp"
#include "core/input_system.hpp"
#include "core/inputs/binding.hpp"
#include "core/window.hpp"
#include "graphics/font.hpp"
#include "graphics/graphics.hpp"
#include "graphics/renderer/renderer_2d.hpp"
#include "graphics/renderer/renderer_3d.hpp"
#include "graphics/texture.hpp"
#include "project/components/camera.hpp"
#include "project/game_object.hpp"
#include "project/project_commands.hpp"
#include "project/project_manager.hpp"
#include "project/scene.hpp"
#include "scripting/backend.hpp"
#include "tools/console/console.hpp"

namespace
{
logger log() { return get_logger("app"); }
} // namespace

application::application()
    : _console(std::make_shared<console>())
{
    project_manager::initialize();

    glfwInit();
    auto main_window = std::make_shared<core::window>();
    main_window->resize(800, 600);
    main_window->set_title("Gamify");
    main_window->init();
    main_window->get_events()->close += [ this ](auto ce)
    {
        shutdown();
        std::erase(_windows, ce.get_sender()->shared_from_this());
    };
    _windows.push_back(main_window);
    main_window->get_events()->render += [ this ](auto re) { render_game(); };
    main_window->get_events()->resize += [ this ](auto re)
    {
        graphics::set_viewport({ 0, 0 }, _windows[ 0 ]->get_size());
        auto cam = components::camera::get_active();
        if (!cam)
            return;

        cam->set_render_size(re.get_new_size());
    };
    core::input_system::set_input_source(main_window);

    scripting::backend::initialize();
    core::input_system::update_device_list();

    common::main_thread_dispatcher::initialize();
    load_assets();
    setup_console();
}

application::~application()
{
    shutdown();
    glfwTerminate();
    scripting::backend::shutdown();
    common::main_thread_dispatcher::shutdown();
}

int application::run()
{
    if (scene::get_active_scene())
    {
        scene::get_active_scene()->visit_root_objects([](auto obj)
        { obj->init(); });
    }

    while (_is_running)
    {
        common::main_thread_dispatcher::run_all();
        update_windows();
        process_console_commands();
        game_clock::frame();
    }
    return 0;
}

void application::shutdown()
{
    _is_running = false;
    scene::get_active_scene()->unload();
    project_manager::shutdown();
    assets::asset_manager::shutdown();
}

void application::setup_console()
{
    cmd_show_mesh::open_window_requested +=
        [ this ](std::shared_ptr<core::window> w)
    {
        _windows.push_back(w);
        w->get_events()->close += [ this ](auto ce)
        { std::erase(_windows, ce.get_sender()->shared_from_this()); };
    };
    cmd_show_material::open_window_requested +=
        [ this ](std::shared_ptr<core::window> w)
    {
        _windows.push_back(w);
        w->get_events()->close += [ this ](auto ce)
        { std::erase(_windows, ce.get_sender()->shared_from_this()); };
    };
    cmd_show_texture::open_window_requested +=
        [ this ](std::shared_ptr<core::window> w)
    {
        _windows.push_back(w);
        w->get_events()->close += [ this ](auto ce)
        { std::erase(_windows, ce.get_sender()->shared_from_this()); };
    };

    cmd_show_profiler::open_window_requested +=
        [ this ](std::shared_ptr<core::window> w)
    {
        _windows.push_back(w);
        w->get_events()->close += [ this ](auto ce)
        { std::erase(_windows, ce.get_sender()->shared_from_this()); };
    };

    _console = std::make_unique<console>();
    _console->register_for_logs();
    _console->register_command<cmd_show_texture, std::string>("show.texture");
    _console->register_command<cmd_show_mesh, std::string>("show.mesh");
    _console->register_command<cmd_show_material, std::string>("show.material");
    _console->register_command<cmd_show_profiler>("show.profiler");
    _console->register_command<cmd_list_textures>("list.textures");
    _console->register_command<project::cmd_create_game_object>(
        "create.gameobject");
    _console->register_command<project::cmd_save_scene, std::string>(
        "save.scene");
    _console->register_command<project::cmd_load_scene, std::string>(
        "load.scene");
    _console->register_command<project::cmd_print_selected_object>(
        "show.selected");
    _console->register_command<project::cmd_rename_object, std::string>(
        "rename");
    _console->register_command<project::cmd_select_object, size_t>("select");
    _console->register_command<project::cmd_list_objects>("list.objects");
    _console->enable_autoactivation();

    project::cmd_load_scene::scene_loaded +=
        [ this ](auto sc) { scene_loaded(sc); };
}

void application::update_windows()
{
    auto p = prof::profile_frame(__PRETTY_FUNCTION__);
    if (scene::get_active_scene())
    {
        scene::get_active_scene()->visit_root_objects([](auto obj)
        { obj->update(); });
    }

    for (int i = 0; i < _windows.size(); ++i)
    {
        auto window = _windows[ i ];
        window->update();
    }
}

void application::process_console_commands()
{
    _console->processor().execute_all();
}

void application::load_assets()
{
    auto project_path = common::filesystem::path::current_dir() / "resources";
    assets::asset_manager::initialize(project_path.full_path());
    assets::asset_manager::scan_project_directory();
    assets::asset_manager::setup_project_directory_watch();
}

void application::render_game()
{
    auto cam = components::camera::get_active();
    if (!cam)
    {
        const auto line_height = 14;
        auto msg_font =
            assets::asset_manager::get<graphics::font>("default.ttf");
        static constexpr std::string_view msg = "No active camera";
        graphics::clear({ 0.0, 0.0, 0.0, 1.0 });
        renderer_2d().draw_text(
            (static_cast<glm::vec2>(_windows[ 0 ]->get_size()) -
             msg_font->size(msg)) /
                2.0f,
            msg_font,
            _windows[ 0 ]->get_size(),
            msg);
    }
    else
    {
        cam->render();
    }

    if (_console && _console->is_active())
    {
        draw_console();
    }
}

void application::draw_console()
{
    auto position = glm::vec2(10, _windows[ 0 ]->get_height() - 10);
    auto fnt = assets::asset_manager::get<graphics::font>("default.ttf");
    auto line_height = fnt->size("X").y + 2;
    auto last_10 = _console->history<10>();
    size_t lines = 0;

    renderer_2d().draw_rect(
        { 0,
          _windows[ 0 ]->get_height() - 20 -
              (last_10.size() + 1) * line_height },
        { _windows[ 0 ]->get_width(), _windows[ 0 ]->get_height() },
        _windows[ 0 ]->get_size(),
        0,
        {},
        { 0, 0, 0, 0.5f });

    for (int i = 0; i < last_10.size(); ++i)
    {
        auto line = last_10[ i ];
        if (line.empty())
            continue;
        ++lines;
        renderer_2d().draw_text(
            { 10, _windows[ 0 ]->get_height() - 10 - ((i + 1) * line_height) },
            assets::asset_manager::get<graphics::font>("default.ttf"),
            _windows[ 0 ]->get_size(),
            line);
    }

    renderer_2d().draw_text(
        { 10, _windows[ 0 ]->get_height() - 10 },
        assets::asset_manager::get<graphics::font>("default.ttf"),
        _windows[ 0 ]->get_size(),
        std::format("> {}", _console->get_current_input()));
}

void application::scene_loaded(std::shared_ptr<scene> sc)
{
    sc->visit_root_objects([](auto obj) { obj->init(); });
}
