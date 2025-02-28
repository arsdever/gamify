#include <QApplication>
#include <QMainWindow>
#include <future>

#include <GLFW/glfw3.h>
#include <assets/asset_manager.hpp>
#include <common/main_thread_dispatcher.hpp>
#include <graphics/graphics_fwd.hpp>
#include <graphics/material.hpp>
#include <qmainwindow.h>

#include "application/qt/material_editor/material_property_ui_builder.hpp"
#include "common/filesystem.hpp"
#include "tools/material_viewer/material_viewer.hpp"

int main(int argc, char** argv)
{
    int exit_code;
    QApplication* app = nullptr;
    QMainWindow* wnd = nullptr;

    auto qtguijob =
        std::packaged_task<void()>([ &argc, argv, &wnd, &exit_code ]()
    {
        QApplication app(argc, argv);
        wnd = new QMainWindow;
        wnd->show();
        exit_code = app.exec();
    });

    auto future = std::move(qtguijob.get_future());
    auto qt_thread = std::thread(std::move(qtguijob));

    common::main_thread_dispatcher::initialize();

    glfwInit();

    std::shared_ptr<material_viewer> viewer =
        std::make_shared<material_viewer>();
    viewer->on_user_initialize +=
        [ &app, &viewer, &main_wnd = wnd ](std::shared_ptr<core::window> wnd)
    {
        assets::asset_manager::initialize(
            (common::filesystem::path::current_dir() / "resources")
                .full_path());
        assets::asset_manager::scan_project_directory();
        assets::asset_manager::setup_project_directory_watch();
        viewer->get_events()->close += [ &app ](auto ce) { app->quit(); };
        viewer->set_mesh_presets({
            assets::asset_manager::try_get<graphics::mesh>("meshes.cube.fbx"),
            assets::asset_manager::try_get<graphics::mesh>("meshes.sphere.fbx"),
            assets::asset_manager::try_get<graphics::mesh>("meshes.shader.fbx"),
        });
        auto mat = assets::asset_manager::try_get<graphics::material>(
            "standard.standard.mat");
        viewer->set_material(mat);

        QMetaObject::invokeMethod(qApp,
                                  [ &main_wnd, mat ]
        {
            auto wdg = material_property_ui_builder::build(mat);
            main_wnd->setCentralWidget(wdg);
        },
                                  Qt::QueuedConnection);
    };
    viewer->init();

    while (future.valid())
    {
        viewer->update();
        common::main_thread_dispatcher::run_all();
    }

    common::main_thread_dispatcher::shutdown();

    return 0;
}
