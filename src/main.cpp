/* clang-format off */
#include <glad/gl.h>
#include <GLFW/glfw3.h>
/* clang-format on */

#include <array>
#include <atomic>
#include <charconv>
#include <chrono>
#include <thread>
#include <unordered_set>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "game_object.hpp"
#include "gl_window.hpp"
#include "logging.hpp"
#include "material.hpp"
#include "scene.hpp"
#include "shader.hpp"
#include "text.hpp"
#include "thread.hpp"

namespace
{
static logger log() { return get_logger("main"); }
unsigned last_fps;
text console_text;
scene s;
std::string console_text_content;
std::unordered_set<int> pressed_keys;
} // namespace

void process_console();
void on_keypress(
    GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
    {
        return;
    }

    switch (key)
    {
    case GLFW_KEY_ENTER:
    {
        process_console();
        break;
    }
    case GLFW_KEY_ESCAPE:
    {
        console_text_content.clear();
        break;
    }
    case GLFW_KEY_BACKSPACE:
    {
        if (!console_text_content.empty())
            console_text_content.pop_back();
        break;
    }
    default:
    {
        if (key >= GLFW_KEY_SPACE && key <= GLFW_KEY_GRAVE_ACCENT)
        {
            if (isalnum(key))
            {
                if (mods & GLFW_MOD_SHIFT)
                {
                    console_text_content.push_back(toupper(key));
                }
                else
                {
                    console_text_content.push_back(tolower(key));
                }
            }
            else
            {
                console_text_content.push_back(key);
            }
        }
    }
    }

    console_text.set_text(console_text_content);
}

std::vector<std::string_view> tokenize(std::string_view str);

void initScene();

void on_error(int error_code, const char* description)
{
    log()->error("Error {}: {}", error_code, description);
}

static std::atomic_int counter = 0;

int main(int argc, char** argv)
{
    glfwInit();
    glfwSetErrorCallback(on_error);

    std::vector<gl_window*> windows;
    windows.push_back(new gl_window);
    windows.back()->init();

    windows[ 0 ]->set_active();
    initScene();

    windows.push_back(new gl_window);
    windows.back()->init();

    console_text_content = "Hello world";
    console_text.set_text(console_text_content);

    // fps counter thread
    logger fps_counter_log = get_logger("fps_counter");
    std::atomic_bool program_exits = false;
    std::thread thd { [ &fps_counter_log, &program_exits ]
    {
        while (!program_exits)
        {
            fps_counter_log->info("FPS: {}", counter);
            last_fps = counter;
            counter = 0;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } };
    set_thread_name(thd, "fps_counter");

    while (true)
    {
        for (auto& window : windows)
        {
            window->update();
        }
    }

    color col { 1, 1, 1, 1 };
    program_exits = true;
    glfwTerminate();
    thd.join();
    return 0;
}

void initScene()
{
    shader_program* prog = new shader_program;
    prog->init();
    prog->add_shader("shader.vert");
    prog->add_shader("shader.frag");
    prog->link();

    material basic_mat;
    basic_mat.set_shader_program(prog);
    for (const auto& property : basic_mat.properties())
    {
        log()->info("Material properties:\n\tname: {}\n\tindex: {}\n\tsize: "
                    "{}\n\ttype: {}",
                    property._name,
                    property._index,
                    property._size,
                    property._type);
    }
    basic_mat.set_property("position", 1);

    game_object* object = new game_object;
    mesh object_mesh;
    object_mesh.init();
    object->set_mesh(std::move(object_mesh));
    object->set_material(std::move(basic_mat));

    s.add_object(object);
}

std::vector<std::string_view> tokenize(std::string_view str)
{
    std::vector<std::string_view> result;
    const char* iter = str.data();
    size_t length = 0;
    for (int i = 0; i < str.size(); ++i)
    {
        if (str[ i ] == ' ')
        {
            result.push_back({ iter, length });
            iter += length + 1;
            length = 0;
            continue;
        }
        length++;
    }

    result.push_back({ iter, length });
    return result;
}

void process_console()
{
    auto tokens = tokenize(console_text_content);
    if (tokens[ 0 ] == "set" && tokens[ 1 ] == "position")
    {
        float x, y;
        std::from_chars(
            tokens[ 2 ].data(), tokens[ 2 ].data() + tokens[ 2 ].size(), x);
        std::from_chars(
            tokens[ 3 ].data(), tokens[ 3 ].data() + tokens[ 3 ].size(), y);

        console_text.set_position({ x, y });
    }
}
