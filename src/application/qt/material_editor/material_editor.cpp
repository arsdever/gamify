#include <QApplication>
#include <QDockWidget>
#include <QMainWindow>
#include <QWindow>
#include <future>

#include <GLFW/glfw3.h>
#include <assets/asset_manager.hpp>
#include <common/main_thread_dispatcher.hpp>
#include <graphics/graphics_fwd.hpp>
#include <graphics/material.hpp>
#include <graphics/texture.hpp>
#include <qmainwindow.h>

#include "application/qt/material_editor/material_property_ui_builder.hpp"
#include "common/filesystem.hpp"
#include "tools/material_viewer/material_viewer.hpp"

int main(int argc, char** argv)
{
    int exit_code;
    QMainWindow* wnd = nullptr;

    QApplication app(argc, argv);
    wnd = new QMainWindow;
    wnd->show();

    common::main_thread_dispatcher::initialize();

    glfwInit();

    std::shared_ptr<material_viewer> viewer =
        std::make_shared<material_viewer>();
    viewer->set_background_color({ 0.14, 0.14, 0.14, 1.0 });
    viewer->on_user_initialize +=
        [ &app, &viewer, &main_wnd = wnd ](std::shared_ptr<core::window> wnd)
    {
        assets::asset_manager::initialize(
            (common::filesystem::path::current_dir() / "resources")
                .full_path());
        assets::asset_manager::scan_project_directory();
        assets::asset_manager::setup_project_directory_watch();
        viewer->get_events()->close += [ &app ](auto ce) { app.quit(); };
        viewer->set_mesh_presets({
            assets::asset_manager::try_get<graphics::mesh>("meshes.cube.fbx"),
            assets::asset_manager::try_get<graphics::mesh>("meshes.sphere.fbx"),
            assets::asset_manager::try_get<graphics::mesh>("meshes.shader.fbx"),
        });

        auto empty_texture =
            assets::asset_manager::get<graphics::texture>("images.empty.png");
        empty_texture->set_active_texture(0);
        auto mat = assets::asset_manager::try_get<graphics::material>(
            "standard.standard.mat");
        auto fb_shader = assets::asset_manager::get<graphics::shader>(
            "standard.fallback.shader");
        graphics::material::set_fallback_shader(fb_shader);
        viewer->set_material(mat);

        auto wdg = material_property_ui_builder::build(mat);
        QDockWidget* dock = new QDockWidget("Properties", main_wnd);
        main_wnd->addDockWidget(Qt::RightDockWidgetArea, dock);
        dock->setWidget(wdg);
    };
    viewer->init();

    QWindow* glwindow = QWindow::fromWinId((WId)(viewer->get_native_handle()));
    auto cwdg = QWidget::createWindowContainer(glwindow);
    wnd->setCentralWidget(cwdg);
    cwdg->setMinimumSize(500, 500);

    while (wnd->isVisible())
    {
        app.processEvents();
        viewer->update();
        auto camera_position = viewer->get_camera_position();
        wnd->setWindowTitle(std::format("{:05.4f}:{:05.4f}:{:05.4f}",
                                        camera_position.x,
                                        camera_position.y,
                                        camera_position.z)
                                .c_str());
        common::main_thread_dispatcher::run_all();
    }

    common::main_thread_dispatcher::shutdown();

    return 0;
}
