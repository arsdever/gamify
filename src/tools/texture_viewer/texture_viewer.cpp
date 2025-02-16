/* clang-format off */
#include <GLFW/glfw3.h>
/* clang-format on */

#include "tools/texture_viewer/texture_viewer.hpp"

#include "assets/asset_manager.hpp"
#include "core/window.hpp"
#include "graphics/graphics.hpp"
#include "graphics/graphics_buffer.hpp"
#include "graphics/material.hpp"
#include "graphics/renderer/renderer_3d.hpp"
#include "graphics/texture.hpp"
#include "project/game_object.hpp"

texture_viewer::texture_viewer()
{
    set_title("Texture Viewer");
    resize(600, 600);
    on_user_initialize +=
        [ this ](std::shared_ptr<core::window>) { initialize(); };
}

texture_viewer::~texture_viewer() = default;

void texture_viewer::set_texture(std::shared_ptr<graphics::texture> m)
{
    _texture = m;
}

void texture_viewer::initialize()
{
    get_events()->render += [ this ](auto e) { render(); };
    get_events()->mouse_release += [ this ](auto e)
    {
        if (e.get_button() == GLFW_MOUSE_BUTTON_1)
        {
            _panning = false;
        }
    };
    get_events()->mouse_press += [ this ](auto e)
    {
        if (e.get_button() == GLFW_MOUSE_BUTTON_1)
        {
            _pan_start = e.get_local_position();
            _panning = true;
        }
    };
    get_events()->mouse_move += [ this ](auto e)
    {
        if (_panning)
        {
            _offset += e.get_local_position() - _pan_start;
            _pan_start = e.get_local_position();
        }
    };
    get_events()->resize +=
        [ this ](auto e) { resize(e.get_new_size().x, e.get_new_size().y); };
    get_events()->mouse_scroll +=
        [ this ](auto e) { _zoom *= std::pow(2.0, e.get_delta().y / 10.0); };
}

void texture_viewer::render()
{
    if (_texture == nullptr)
        return;

    auto mat =
        assets::asset_manager::get<graphics::material>("standard.surface.mat");
    auto quad = assets::asset_manager::get<graphics::mesh>("meshes.quad.fbx");
    glm::mat4 camera_matrix = glm::ortho(
        -1.0 / _zoom, 1.0 / _zoom, -1.0 / _zoom, 1.0 / _zoom, .001, 100.0);
    glm::mat4 camera_transform = glm::identity<glm::mat4>();
    camera_transform =
        glm::translate(camera_transform,
                       glm::vec3(-_offset.x / _zoom * 2 / get_width(),
                                 _offset.y / _zoom * 2 / get_height(),
                                 1));
    glm::mat4 model_matrix = glm::rotate(
        glm::identity<glm::mat4>(), glm::radians(90.0f), glm::vec3(1, 0, 0));
    mat->set_property_value("u_vp_matrix",
                            camera_matrix * glm::inverse(camera_transform));
    mat->set_property_value("u_model_matrix", model_matrix);
    mat->set_property_value("u_color", glm::vec4 { 1.0f });
    mat->set_property_value("u_image", _texture);

    graphics::set_viewport({ 0, 0 }, { get_size() });
    graphics::clear({ glm::vec3(0.1f), 1.0f });

    renderer_3d().draw_mesh(quad, mat);
}
