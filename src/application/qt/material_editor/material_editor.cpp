#include <QApplication>
#include <QMainWindow>
#include <QSlider>
#include <QThread>
#include <iostream>

#include <GLFW/glfw3.h>
#include <assets/asset_manager.hpp>
#include <common/main_thread_dispatcher.hpp>
#include <graphics/graphics_fwd.hpp>
#include <graphics/material.hpp>

#include "common/filesystem.hpp"
#include "tools/material_viewer/material_viewer.hpp"

int main(int argc, char** argv)
{
    int exit_code;
    QApplication* app = nullptr;
    QSlider* spin_box = nullptr;

    auto qtguijob =
        std::packaged_task<void()>([ &argc, argv, &exit_code, &spin_box ]()
    {
        QApplication app(argc, argv);
        QMainWindow wnd;
        spin_box = new QSlider(Qt::Horizontal);
        wnd.setCentralWidget(spin_box);
        spin_box->setMinimum(0);
        spin_box->setMaximum(100);
        spin_box->setSingleStep(1);
        wnd.show();
        exit_code = app.exec();
    });

    auto future = std::move(qtguijob.get_future());
    auto qt_thread = std::thread(std::move(qtguijob));

    common::main_thread_dispatcher::initialize();

    glfwInit();

    std::shared_ptr<material_viewer> viewer =
        std::make_shared<material_viewer>();
    viewer->on_user_initialize +=
        [ &app, &viewer, &spin_box ](std::shared_ptr<core::window> wnd)
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

        if (spin_box)
        {
            spin_box->setValue(
                mat->get_property_value<float>("u_roughness").value_or(0.0f));
            spin_box->connect(spin_box,
                              &QSlider::valueChanged,
                              [ mat, &spin_box ](int value)
            { mat->set_property_value("u_roughness", value / 100.0f); });
        }
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
