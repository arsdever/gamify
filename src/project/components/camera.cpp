#include <GLFW/glfw3.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <nlohmann/json.hpp>

#include "project/components/camera.hpp"

#include "assets/asset_manager.hpp"
#include "common/logging.hpp"
#include "core/settings.hpp"
#include "core/window.hpp"
#include "graphics/framebuffer.hpp"
#include "graphics/graphics.hpp"
#include "graphics/graphics_buffer.hpp"
#include "graphics/material.hpp"
#include "graphics/renderer/renderer_3d.hpp"
#include "graphics/texture.hpp"
#include "project/components/light.hpp"
#include "project/components/mesh_filter.hpp"
#include "project/components/mesh_renderer.hpp"
#include "project/components/transform.hpp"
#include "project/project_manager.hpp"
#include "project/scene.hpp"
#include "project/serialization_utilities.hpp"
#include "project/serializer.hpp"
#include "project/serializer_json.hpp"

using namespace serialization::utilities;

namespace components
{
camera::camera(game_object& obj)
    : component("camera", obj)
{
    _cameras.push_back(this);
}

camera& camera::operator=(camera&& obj) = default;

camera::~camera() { }

void camera::set_fov(double fov)
{
    _field_of_view = fov;
    _projection_matrix_dirty = true;
}

double camera::get_fov() const { return _field_of_view; }

void camera::set_orthogonal(bool ortho_flag)
{
    _is_orthogonal = ortho_flag;
    _projection_matrix_dirty = true;
}

bool camera::is_orthogonal() const { return _is_orthogonal; }

double camera::get_aspect_ratio() const
{
    auto drs = glm::dvec2(get_render_size());
    return drs.x / drs.y;
}

void camera::set_active() { _active_camera = this; }

camera* camera::get_active() { return _active_camera; }

std::vector<camera*> camera::all() { return _cameras; }

void camera::set_render_size(size_t width, size_t height)
{
    set_render_size({ width, height });
}

void camera::set_render_size(glm::uvec2 size)
{
    if (_render_size != size)
    {
        _render_size = glm::max(glm::uvec2 { 1, 1 }, size);
        _framebuffer->resize(_render_size);
        _projection_matrix_dirty = true;
    }
}

void camera::get_render_size(size_t& width, size_t& height) const
{
    width = _render_size.x;
    height = _render_size.y;
}

glm::uvec2 camera::get_render_size() const { return _render_size; }

void camera::set_render_texture(std::weak_ptr<graphics::texture> render_texture)
{
    _user_render_texture = render_texture;
}

std::shared_ptr<graphics::texture> camera::get_render_texture() const
{
    return _user_render_texture.lock();
}

void camera::set_background_color(glm::dvec4 color)
{
    _background_color = color;
}

glm::dvec4 camera::get_background_color() const
{
    return glm::dvec4(_background_color);
}

void camera::render()
{
    graphics::set_viewport({ 0, 0 }, _render_size);
    setup_lights();
    render_on_private_texture();

    _framebuffer->bind();
    render_texture_background();

    // if (get_gizmos_enabled())
    // {
    //     render_gizmos();
    // }

    _framebuffer->unbind();
    if (auto urt = _user_render_texture.lock())
    {
        _framebuffer->copy_texture(urt);
    }

    _framebuffer->blit(0);
    // texture surface;
    // surface.init(_render_size.x,
    // _render_size.y,graphics::texture::format::RGBA);
    // _framebuffer->copy_texture(&surface);
    // renderer_2d().draw_rect(glm::vec2(0),
    //                         glm::vec2(_render_size),
    //                         glm::vec2(_render_size),
    //                         surface);
}

glm::mat4 camera::projection_matrix() const { return _projection_matrix; }

glm::mat4 camera::view_matrix() const { return _view_matrix; }

glm::mat4 camera::vp_matrix() const
{
    return projection_matrix() * view_matrix();
}

template <>
void camera::serialize<json_serializer>(json_serializer& s)
{
    s.add_component(nlohmann::json {
        { "type", camera::type_name },
        { "is_enabled", is_enabled() },
    });
}

void camera::deserialize(const nlohmann::json& j)
{
    set_enabled(j[ "is_enabled" ]);
}

void camera::on_init()
{
    _view_matrix = glm::inverse(get_transform()->get_matrix());
    _projection_matrix = calculate_projection_matrix();

    _framebuffer = std::make_unique<framebuffer>();
    _framebuffer->set_samples(core::settings[ "antialiasing" ].get<unsigned>());

    auto wnd = core::window::get_main_window();
    set_render_size(wnd->get_size());

    _framebuffer->initialize();
    _lights_buffer = std::make_unique<graphics_buffer>(
        graphics_buffer::type::shader_storage);
}

void camera::on_update()
{
    _view_matrix = glm::inverse(get_transform()->get_matrix());

    if (_projection_matrix_dirty)
    {
        _projection_matrix = calculate_projection_matrix();
        _projection_matrix_dirty = false;
    }
}

void camera::set_projection_matrix(glm::mat4 mat)
{
    _projection_matrix = std::move(mat);
}

void camera::set_view_matrix(glm::mat4 mat) { _view_matrix = std::move(mat); }

void camera::render_texture_background()
{
    auto mat =
        assets::asset_manager::get<graphics::material>("standard.skybox.mat");
    mat->set_property_value("u_model_matrix", glm::identity<glm::mat4>());
    mat->set_property_value("u_vp_matrix", vp_matrix());
    renderer_3d().draw_mesh(
        assets::asset_manager::get<graphics::mesh>("meshes.env_sphere.fbx"),
        mat);
}

void camera::render_on_private_texture() const
{
    _framebuffer->bind();
    graphics::clear(_background_color);

    if (scene::get_active_scene())
    {
        scene::get_active_scene()->visit_root_objects([ this ](auto& obj)
        {
            if (!obj->is_active())
                return;

            auto render_action = [ this ](auto& obj)
            {
                if (auto renderer =
                        obj->template try_get<components::mesh_renderer>())
                {
                    auto mesh =
                        obj->template get<components::mesh_filter>().get_mesh();
                    auto material = renderer->get_material();
                    if (material && mesh)
                    {
                        material->set_property_value(
                            "u_model_matrix",
                            glm::mat4(obj->get_transform().get_matrix()));
                        material->set_property_value("u_vp_matrix",
                                                     vp_matrix());
                        material->set_property_value(
                            "u_camera_position",
                            glm::vec3(get_transform()->get_position()));
                        renderer_3d().draw_mesh(mesh, material);
                    }
                }
                return true;
            };

            obj->visit_children(render_action);
            render_action(obj);
        });
    }
    _framebuffer->unbind();
}

void camera::setup_lights()
{
    struct glsl_lights_t
    {
        glm::vec3 position;
        float intensity;
        glm::vec3 direction;
        float radius;
        glm::vec3 color;
        uint32_t type;
    };

    std::vector<glsl_lights_t> glsl_lights;
    size_t i = 0;

    scene::get_active_scene()->visit_root_objects([ &i, &glsl_lights ](auto obj)
    {
        if (auto light = obj->template try_get<components::light>())
        {
            glsl_lights.push_back({});
            auto& glight = glsl_lights.back();
            glight.position = light->get_transform()->get_position();
            glight.direction =
                glm::normalize(light->get_transform()->get_rotation() *
                               glm::dvec3 { 0, 0, 1 });
            glight.color = light->get_color();
            glight.intensity = light->get_intensity();
            glight.radius = light->get_radius();
            glight.type = static_cast<uint32_t>(light->get_type());
            ++i;
        }
    });

    _lights_buffer->set_element_stride(4 * 3 * sizeof(float));
    _lights_buffer->set_element_count(glsl_lights.size());
    _lights_buffer->set_usage_type(graphics_buffer::usage_type::dynamic_copy);
    _lights_buffer->set_data(glsl_lights.data());
    _lights_buffer->bind(0);
}

glm::mat4 camera::calculate_projection_matrix() const
{
    // TODO: optimize with caching
    // copy into floating point vec2
    glm::dvec2 size = _render_size;
    if (_is_orthogonal)
    {
        glm::dquat rotation = get_transform()->get_rotation();
        glm::dvec3 direction = rotation * glm::dvec3 { 0, 0, 1 };
        float dist =
            std::abs(glm::dot(direction, get_transform()->get_position()));

        return glm::ortho(-size.x / dist,
                          size.x / dist,
                          -size.y / dist,
                          size.y / dist,
                          0.01,
                          10000.0);
    }

    return glm::perspective(_field_of_view, size.x / size.y, 0.1, 10000.0);
}

camera* camera::_active_camera { nullptr };

std::vector<camera*> camera::_cameras;
} // namespace components
