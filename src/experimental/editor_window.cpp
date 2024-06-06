#include <optional>

#include "experimental/editor_window.hpp"

#include "asset_manager.hpp"
#include "experimental/input_system.hpp"
#include "renderer_3d.hpp"

namespace experimental
{

editor_window::editor_window()
{
    on_user_initialize += [ this ](auto) { initialize(); };
}

editor_window::~editor_window() { }

void editor_window::initialize()
{
    asset_manager::default_asset_manager()->load_asset(
        "resources/standard/grid.shader");
    asset_manager::default_asset_manager()->load_asset(
        "resources/standard/line.shader");

    _camera_position = { 5, 5, 5 };
    _camera_direction = glm::normalize(glm::vec3 { 1, 1, 1 });

    input_system::on_keypress += [ this ](int keycode)
    {
        // control local camera movement
        switch (keycode)
        {
        // case GLFW_KEY_D:
        // {
        //     _camera_position.x += .1;
        //     break;
        // }
        // case GLFW_KEY_A:
        // {
        //     _camera_position.x -= .1;
        //     break;
        // }
        // case GLFW_KEY_SPACE:
        // {
        //     _camera_position.y += .1;
        //     break;
        // }
        // case GLFW_KEY_LEFT_CONTROL:
        // {
        //     _camera_position.y -= .1;
        //     break;
        // }
        case GLFW_KEY_W:
        {
            _camera_position -= _camera_direction * .07f;
            break;
        }
        case GLFW_KEY_S:
        {
            _camera_position += _camera_direction * .07f;
            break;
        }
        }
    };
    get_events().render += [ this ](auto re)
    {
        render_grid();
        render_axis();
    };
    get_events().mouse_move += [ this ](auto me)
    {
        if (!get_has_grab())
        {
            _mouse_position = std::nullopt;
            return;
        }

        if (!_mouse_position)
        {
            _mouse_position = me.get_local_position();
            return;
        }

        auto diff = me.get_local_position() - _mouse_position.value();
        _mouse_position = me.get_local_position();

        _camera_direction =
            glm::normalize(glm::rotate(glm::rotate(glm::identity<glm::quat>(),
                                                   glm::radians(diff.x),
                                                   glm::vec3(0, 1, 0)),
                                       glm::radians(-diff.y),
                                       glm::vec3(1, 0, 0)) *
                           _camera_direction);
    };
}

void editor_window::render_grid()
{
    auto camera_right =
        glm::normalize(glm::cross(_camera_direction, { 0, 1, 0 }));
    auto camera_up =
        glm::normalize(glm::cross(camera_right, _camera_direction));
    auto inverse_view_from_origin =
        glm::mat4(glm::mat3(camera_right, camera_up, _camera_direction));
    auto inverse_view =
        glm::translate(glm::identity<glm::mat4>(), _camera_position) *
        inverse_view_from_origin;
    auto view = glm::inverse(inverse_view);
    auto projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);

    glViewport(0, 0, get_width(), get_height());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    renderer_3d::draw_grid(0.3f, glm::identity<glm::mat4>(), view, projection);
}

void editor_window::render_axis()
{
    auto camera_right =
        glm::normalize(glm::cross(_camera_direction, { 0, 1, 0 }));
    auto camera_up =
        glm::normalize(glm::cross(camera_right, _camera_direction));
    auto inverse_view_from_origin =
        glm::mat4(glm::mat3(camera_right, camera_up, _camera_direction));
    auto inverse_view =
        glm::translate(glm::identity<glm::mat4>(), _camera_position) *
        inverse_view_from_origin;
    auto view = glm::inverse(inverse_view);
    auto projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);

    unsigned axis_viewport_size = 80;
    glViewport(get_width() - axis_viewport_size - 20,
               get_height() - axis_viewport_size - 20,
               axis_viewport_size,
               axis_viewport_size);
    view = glm::inverse(inverse_view_from_origin);

    renderer_3d::draw_ray(glm::vec3 { -_camera_direction },
                          glm::vec3 { 1, 0, 0 },
                          0.3f,
                          { 1, 0, 0, 1 },
                          view,
                          projection);
    renderer_3d::draw_ray(glm::vec3 { -_camera_direction },
                          glm::vec3 { 0, 1, 0 },
                          0.3f,
                          { 0, 1, 0, 1 },
                          view,
                          projection);
    renderer_3d::draw_ray(glm::vec3 { -_camera_direction },
                          glm::vec3 { 0, 0, 1 },
                          0.3f,
                          { 0, 0, 1, 1 },
                          view,
                          projection);
}

} // namespace experimental
