/* clang-format off */
#include <glad/gl.h>
#include <GLFW/glfw3.h>
/* clang-format on */

#include <glm/ext.hpp>

#include "gl_window.hpp"

#include "camera.hpp"
#include "game_object.hpp"
#include "logging.hpp"
#include "scene.hpp"
#include "shader.hpp"

namespace
{
static inline logger log() { return get_logger("window"); }
static inline void gl_debug_output(GLenum source,
                                   GLenum type,
                                   unsigned int id,
                                   GLenum severity,
                                   GLsizei length,
                                   const char* message,
                                   const void* userParam)
{
    // SOURCE: https://learnopengl.com/In-Practice/Debugging
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;

    log()->error("---------------");
    log()->error("Debug message ({}): {}", id, message);

    switch (source)
    {
    case GL_DEBUG_SOURCE_API: log()->error("Source: API"); break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        log()->error("Source: Window System");
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        log()->error("Source: Shader Compiler");
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        log()->error("Source: Third Party");
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        log()->error("Source: Application");
        break;
    case GL_DEBUG_SOURCE_OTHER: log()->error("Source: Other"); break;
    }

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR: log()->error("Type: Error"); break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        log()->error("Type: Deprecated Behaviour");
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        log()->error("Type: Undefined Behaviour");
        break;
    case GL_DEBUG_TYPE_PORTABILITY: log()->error("Type: Portability"); break;
    case GL_DEBUG_TYPE_PERFORMANCE: log()->error("Type: Performance"); break;
    case GL_DEBUG_TYPE_MARKER: log()->error("Type: Marker"); break;
    case GL_DEBUG_TYPE_PUSH_GROUP: log()->error("Type: Push Group"); break;
    case GL_DEBUG_TYPE_POP_GROUP: log()->error("Type: Pop Group"); break;
    case GL_DEBUG_TYPE_OTHER: log()->error("Type: Other"); break;
    }

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH: log()->error("Severity: high"); break;
    case GL_DEBUG_SEVERITY_MEDIUM: log()->error("Severity: medium"); break;
    case GL_DEBUG_SEVERITY_LOW: log()->error("Severity: low"); break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        log()->error("Severity: notification");
        break;
    }
}
} // namespace

void gl_window::init()
{
    std::string title = "Window";
    if (!_main_window)
    {
        _main_window = this;
        _is_main_window = true;
        title = "Main window";
    }

    _width = 800;
    _height = 600;

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    glfwWindowHint(GLFW_SAMPLES, 8);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    _window =
        glfwCreateWindow(width(),
                         height(),
                         title.c_str(),
                         NULL,
                         _is_main_window ? nullptr : _main_window->_window);
    glfwSetWindowUserPointer(_window, this);
    if (_window == nullptr)
    {
        log()->error("Failed to create GLFW window");
        return;
    }

    set_active();

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
    {
        log()->error("Failed to initialize GLAD");
        return;
    }

    glViewport(0, 0, width(), height());
    glfwSetFramebufferSizeCallback(_window,
                                   [](GLFWwindow* window, int w, int h)
    {
        gl_window* _this =
            static_cast<gl_window*>(glfwGetWindowUserPointer(window));
        _this->_width = w;
        _this->_height = h;
        _this->_view_camera->set_render_size(w, h);
    });

    {
        // configure gl debug output
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(gl_debug_output, nullptr);
        glDebugMessageControl(
            GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    configure_fps_text();
    _last_frame_time = std::chrono::steady_clock::now();

    _state = state::initialized;
}

void gl_window::set_active() { glfwMakeContextCurrent(_window); }

size_t gl_window::width() const { return _width; }

size_t gl_window::height() const { return _height; }

void gl_window::update()
{
    if (_state != state::initialized)
    {
        // TODO: notify that the window was closed
        return;
    }

    if (glfwWindowShouldClose(_window))
    {
        _state = state::closed;
        glfwDestroyWindow(_window);
        _window = nullptr;
        return;
    }

    set_active();
    glViewport(0, 0, width(), height());
    _view_camera->set_active();
    draw();

    glfwSwapBuffers(_window);
    glfwPollEvents();
}

void gl_window::draw()
{
    auto now = std::chrono::steady_clock::now();
    auto diff = now - _last_frame_time;
    _last_frame_time = std::move(now);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // glfwMakeContextCurrent(_main_window->_window);
    for (auto* obj : scene::get_active_scene().objects())
    {
        obj->update();
    }
    glfwMakeContextCurrent(_window);

    // draw fps counter
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);
    _fps_text.set_text(fmt::format(
        "{:#6.6} ms\nhandle {:#x}",
        std::chrono::duration_cast<std::chrono::duration<double>>(diff)
                .count() *
            1000,
        reinterpret_cast<unsigned long long>(this)));
    _fps_text.render();
    glDisable(GL_BLEND);
}

void gl_window::set_camera(camera* view_camera)
{
    _view_camera = view_camera;
    _view_camera->set_render_size(width(), height());
}

void gl_window::configure_fps_text()
{
    font fps_text_font;
    fps_text_font.load("font.ttf", 9);

    shader_program prog;
    prog.init();
    prog.add_shader("text.vert");
    prog.add_shader("text.frag");
    prog.link();

    prog.use();
    glm::mat4 projection = glm::ortho(
        0.0f, static_cast<float>(width()), 0.0f, static_cast<float>(height()));
    unsigned int uniform_position =
        glGetUniformLocation(prog.id(), "projection");
    glUniformMatrix4fv(
        uniform_position, 1, GL_FALSE, glm::value_ptr(projection));
    prog.unuse();

    _fps_text.init();
    _fps_text.set_position({ 5.0f, height() - 10.0f });
    _fps_text.set_color({ 1.0f, 1.0f, 1.0f });
    _fps_text.set_font(std::move(fps_text_font));
    _fps_text.set_scale(1);
    _fps_text.set_shader(std::move(prog));
}

gl_window* gl_window::_main_window = nullptr;