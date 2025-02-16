/* clang-format off */
#include <GLFW/glfw3.h>
/* clang-format on */

#include "tools/mesh_viewer/mesh_viewer.hpp"

#include "assets/asset_manager.hpp"
#include "core/window.hpp"
#include "graphics/graphics.hpp"
#include "graphics/graphics_buffer.hpp"
#include "graphics/material.hpp"
#include "graphics/mesh.hpp"
#include "graphics/renderer/renderer_3d.hpp"
#include "project/game_object.hpp"

mesh_viewer::mesh_viewer()
{
    set_title("Mesh Viewer");
    resize(600, 600);
    on_user_initialize +=
        [ this ](std::shared_ptr<core::window>) { initialize(); };
}

mesh_viewer::~mesh_viewer() = default;

void mesh_viewer::set_mesh(std::shared_ptr<graphics::mesh> m) { _mesh = m; }

void mesh_viewer::set_mode(unsigned m) { _mode = m; }

void mesh_viewer::initialize()
{
    struct light_t
    {
        glm::vec3 position;
        float intensity;
        glm::vec3 direction;
        float radius;
        glm::vec3 color;
        uint32_t type;
    } l[ 2 ];

    l[ 0 ].position = glm::vec3(3, 3, 3);
    l[ 0 ].color = glm::vec3(1, 1, 1);
    l[ 0 ].intensity = 10;

    l[ 1 ].position = glm::vec3(-3, -3, 3);
    l[ 1 ].color = glm::vec3(1, 1, 1);
    l[ 1 ].intensity = 15;

    _light_buffer = std::make_unique<graphics_buffer>(
        graphics_buffer::type::shader_storage);
    _light_buffer->set_element_stride(12 * sizeof(float));
    _light_buffer->set_element_count(2);
    _light_buffer->set_data(&l);

    get_events()->render += [ this ](auto e) { render(); };
    get_events()->key_press += [ this ](auto e)
    {
        if (e.get_key() == GLFW_KEY_W)
            _draw_wireframe = !_draw_wireframe;

        if (e.get_key() >= '0' && e.get_key() <= '5')
        {
            set_mode(e.get_key() - '0');
        }
    };
    get_events()->mouse_press +=
        [ this ](auto e) { _rotation_start_point = e.get_local_position(); };
    get_events()->mouse_move += [ this ](auto e)
    {
        if (e.get_buttons() & (1 << GLFW_MOUSE_BUTTON_1))
        {
            _rotation.x -= e.get_local_position().y - _rotation_start_point.y;
            _rotation.y -= e.get_local_position().x - _rotation_start_point.x;
            _rotation_start_point = e.get_local_position();
        }
    };
    get_events()->resize +=
        [ this ](auto e) { resize(e.get_new_size().x, e.get_new_size().y); };
    get_events()->mouse_scroll +=
        [ this ](auto e) { _zoom *= std::pow(2.0, e.get_delta().y / 10.0); };
}

void mesh_viewer::render()
{
    _light_buffer->bind(0);

    auto mat = assets::asset_manager::get<graphics::material>(
        "standard.mesh_viewer.mat");
    auto camera_position = glm::vec3(0, 0, _zoom * 8);

    glm::mat4 camera_matrix =
        glm::perspective(glm::radians(30.0f),
                         static_cast<float>(get_width()) /
                             static_cast<float>(get_height()),
                         0.1f,
                         100.0f) *
        glm::lookAt(camera_position, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 model = glm::identity<glm::mat4>();
    model = glm::rotate(model, _rotation.y * 0.01f, glm::vec3(0, 1, 0));
    model = glm::rotate(model, _rotation.x * 0.01f, glm::vec3(1, 0, 0));
    camera_matrix = camera_matrix * glm::inverse(model);
    mat->set_property_value("u_vp_matrix", camera_matrix);
    mat->set_property_value("u_model_matrix", glm::identity<glm::mat4>());
    mat->set_property_value("u_camera_position", camera_position);
    mat->set_property_value("u_mode", _mode);

    mat->set_property_value("u_model_matrix", glm::identity<glm::mat4>());
    graphics::set_viewport({ 0, 0 }, { get_size() });
    graphics::clear({ 0.0f, 0.15f, 0.2f, 1.0f });

    if (_mesh == nullptr)
        return;

    renderer_3d().draw_mesh(_mesh, mat);
    if (_draw_wireframe)
    {
        graphics::set_wireframe(true);
        mat->set_property_value("u_wireframe", 1.0f);
        renderer_3d().draw_mesh(_mesh, mat);
        graphics::set_wireframe(false);
        mat->set_property_value("u_wireframe", 0.0f);
    }
}
