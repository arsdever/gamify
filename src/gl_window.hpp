#pragma once

#include <chrono>
#include <unordered_map>

#include "text.hpp"

struct GLFWwindow;
class camera;

class gl_window
{
    enum class state
    {
        uninitialized,
        initialized,
        closed,
    };

public:
    void init();
    void set_active();

    size_t width() const;
    size_t height() const;

    void update();
    void draw();

    void set_camera(camera* view_camera);

private:
    void configure_fps_text();

private:
    GLFWwindow* _window = nullptr;
    state _state = state::uninitialized;
    size_t _width = 800;
    size_t _height = 600;
    std::chrono::steady_clock::time_point _last_frame_time;
    text _fps_text;
    bool _is_main_window = false;
    camera* _view_camera = nullptr;
    static gl_window* _main_window;
};