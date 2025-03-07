/* clang-format off */
#include <GLFW/glfw3.h>
/* clang-format on */

#include "tools/material_viewer/material_viewer.hpp"

#include "assets/asset_manager.hpp"
#include "core/window.hpp"
#include "graphics/graphics.hpp"
#include "graphics/graphics_buffer.hpp"
#include "graphics/material.hpp"
#include "graphics/renderer/renderer_3d.hpp"
#include "project/game_object.hpp"

material_viewer::material_viewer()
{
    set_title("Material Viewer");
    resize(600, 600);
    on_user_initialize +=
        [ this ](std::shared_ptr<core::window>) { initialize(); };
}

material_viewer::~material_viewer() = default;

void material_viewer::set_material(std::shared_ptr<graphics::material> m)
{
    _material = m;
}

void material_viewer::set_mesh(std::shared_ptr<graphics::mesh> m) { _mesh = m; }

void material_viewer::set_background_color(glm::vec4 color)
{
    _background_color = color;
}

void material_viewer::initialize()
{
    struct light_t
    {
        glm::vec3 position;
        float intensity;
        glm::vec3 direction;
        float radius;
        glm::vec3 color;
        uint32_t type;
    };

    static constexpr auto lights_count = 2;
    std::array<light_t, lights_count> l;

    l[ 0 ].position = glm::vec3(3, 3, 3);
    l[ 0 ].color = glm::vec3(1, 1, 1);
    l[ 0 ].intensity = 10;

    l[ 1 ].position = glm::vec3(-3, -3, 3);
    l[ 1 ].color = glm::vec3(1, 1, 1);
    l[ 1 ].intensity = 15;

    _light_buffer = std::make_unique<graphics_buffer>(
        graphics_buffer::type::shader_storage);
    _light_buffer->set_element_stride(12 * sizeof(float));
    _light_buffer->set_element_count(l.size());
    _light_buffer->set_data(&l);
    _light_buffer->set_usage_type(graphics_buffer::usage_type::dynamic_draw);

    get_events()->render += [ this ](auto e) { render(); };
    get_events()->key_press += [ this ](auto e)
    {
        if (e.get_key() == GLFW_KEY_W)
            _draw_wireframe = !_draw_wireframe;
    };
    get_events()->mouse_press += [ this ](auto e)
    {
        _rotation_start_point = e.get_local_position();
        _camera_rotation_start_point = e.get_local_position();
    };
    get_events()->mouse_move += [ this ](auto e)
    {
        if (e.get_buttons() & (1 << GLFW_MOUSE_BUTTON_1))
        {
            _rotation.x -= e.get_local_position().y - _rotation_start_point.y;
            _rotation.y -= e.get_local_position().x - _rotation_start_point.x;
            _rotation_start_point = e.get_local_position();
        }
        if (e.get_buttons() & (1 << GLFW_MOUSE_BUTTON_2))
        {
            _camera_rotation.x -=
                e.get_local_position().y - _camera_rotation_start_point.y;
            _camera_rotation.y -=
                e.get_local_position().x - _camera_rotation_start_point.x;
            _camera_rotation_start_point = e.get_local_position();
        }
    };
    get_events()->resize +=
        [ this ](auto e) { resize(e.get_new_size().x, e.get_new_size().y); };
    get_events()->mouse_scroll +=
        [ this ](auto e) { _zoom *= std::pow(2.0, e.get_delta().y / 10.0); };
}

glm::mat4 material_viewer::get_camera_matrix() const
{
    glm::mat4 camera_matrix = glm::identity<glm::mat4>();
    camera_matrix = glm::rotate(
        camera_matrix, _camera_rotation.y * 0.01f, glm::vec3(0, 1, 0));
    camera_matrix = glm::rotate(
        camera_matrix, _camera_rotation.x * 0.01f, glm::vec3(1, 0, 0));
    camera_matrix = glm::translate(camera_matrix, glm::vec3(0, 0, _zoom * 4));
    return camera_matrix;
}

glm::vec3 material_viewer::get_camera_position() const
{
    return get_camera_matrix() * glm::vec4(0, 0, 0, 1);
}

std::shared_ptr<graphics::material> material_viewer::get_material() const
{
    return _material;
}

std::shared_ptr<graphics::mesh> material_viewer::get_mesh() const
{
    return _mesh;
}

void material_viewer::render()
{
    _light_buffer->bind(0);

    if (_mesh == nullptr)
    {
        return;
    }

    if (_mesh == nullptr)
        return;

    glm::mat4 model = glm::identity<glm::mat4>();
    model = glm::rotate(model, _rotation.y * 0.01f, glm::vec3(0, 1, 0));
    model = glm::rotate(model, _rotation.x * 0.01f, glm::vec3(1, 0, 0));

    auto camera_matrix = get_camera_matrix();
    glm::vec3 camera_position = camera_matrix * glm::vec4(0, 0, 0, 1);
    camera_matrix = glm::perspective(glm::radians(30.0f),
                                     static_cast<float>(get_width()) /
                                         static_cast<float>(get_height()),
                                     0.1f,
                                     100.0f) *
                    glm::inverse(camera_matrix);
    // glm::inverse(camera_matrix);

    _material->set_property_value("u_vp_matrix", camera_matrix);
    _material->set_property_value("u_model_matrix", model);
    _material->set_property_value("u_camera_position", camera_position);

    graphics::set_viewport({ 0, 0 }, { get_size() });
    graphics::clear(_background_color);

    if (_material == nullptr)
        return;

    renderer_3d().draw_mesh(_mesh, _material);
}
