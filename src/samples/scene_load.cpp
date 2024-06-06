/* clang-format off */
#include <glad/gl.h>
#include <GLFW/glfw3.h>
/* clang-format on */

#include <memory>

#include "asset_manager.hpp"
#include "camera.hpp"
#include "experimental/viewport.hpp"
#include "experimental/window.hpp"
#include "feature_flags.hpp"
#include "gl_error_handler.hpp"
#include "logging.hpp"

namespace
{
logger log() { return get_logger("window example"); }
} // namespace

void init_scene();

int main(int argc, char** argv)
{
    glfwInit();
    std::vector<std::shared_ptr<experimental::window>> windows;
    auto exp_window = std::make_shared<experimental::window>();

    std::shared_ptr<camera> main_camera = nullptr;

    feature_flags::set_flag(feature_flags::flag_name::load_fbx_as_scene, true);

    exp_window->on_user_initialize +=
        [ &main_camera ](std::shared_ptr<experimental::window> wnd)
    {
        // configure gl debug output
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(gl_error_handler, nullptr);
        glDebugMessageControl(
            GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

        auto* am = asset_manager::default_asset_manager();
        am->load_asset("standard.shader");
        am->load_asset("shader_ball.fbx");

        main_camera = std::shared_ptr<camera>(camera::all_cameras()[ 0 ]);
        main_camera->set_active();
        init_scene();
        auto vp = std::make_shared<experimental::viewport>();
        vp->set_camera(main_camera);
        vp->set_size(wnd->get_size());
        wnd->add_viewport(vp);
    };

    exp_window->set_title("Hello experimental window");
    exp_window->resize(800, 800);
    exp_window->init();

    exp_window->set_can_grab(true);

    exp_window->get_events().close += [ &windows ](auto ce)
    { std::erase(windows, ce.get_sender()->shared_from_this()); };

    exp_window->get_events().resize += [](auto re)
    {
        auto wnd = re.get_sender();
        if (auto vp = wnd->get_viewports()[ 0 ])
        {
            vp->set_size(wnd->get_size());
        }
    };

    int frame_counter = 0;
    exp_window->get_events().render += [ &frame_counter ](auto re)
    {
        if (frame_counter % 30 == 0)
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            int i = 0;
            for (auto vp : re.get_sender()->get_viewports())
            {
                vp->render();
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

    return 0;
}

void init_scene()
{
    camera::active_camera()->set_background(glm::vec3(.3, .6, .7));
}
