#include <glm/gtx/matrix_decompose.hpp>
#include <nlohmann/json.hpp>

#include "project/components/camera.hpp"

#include "project/components/transform.hpp"
#include "project/memory_manager.hpp"
#include "project/serialization_utilities.hpp"
#include "project/serializer.hpp"
#include "project/serializer_json.hpp"

using namespace serialization::utilities;

namespace components
{
camera::camera(game_object& obj)
    : component("camera", obj)
{
}

camera::~camera() { std::cout << "~camera" << std::endl; }

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
    _render_size = glm::max(glm::uvec2 { 1, 1 }, size);
    _projection_matrix_dirty = true;
}

void camera::get_render_size(size_t& width, size_t& height) const
{
    width = _render_size.x;
    height = _render_size.y;
}

glm::uvec2 camera::get_render_size() const { return _render_size; }

void camera::set_render_texture(std::weak_ptr<texture> render_texture)
{
    _user_render_texture = render_texture;
}

std::shared_ptr<texture> camera::get_render_texture() const
{
    return _user_render_texture.lock();
}

void camera::set_background(glm::dvec4 color) { _background_color = color; }

void camera::set_background(std::weak_ptr<texture> img)
{
    _background_texture = img;
}

glm::dvec4 camera::get_background_color() const
{
    return glm::dvec4(_background_color);
}

std::shared_ptr<texture> camera::get_background_texture() const
{
    return _background_texture.lock();
}

void camera::render()
{
    glClearColor(_background_color.x,
                 _background_color.y,
                 _background_color.z,
                 _background_color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
        { "type", type_id<camera>() },
        { "is_enabled", is_enabled() },
    });
}

void camera::deserialize(const nlohmann::json& j)
{
    set_enabled(j[ "is_enabled" ]);
}

void camera::on_update()
{
    if (get_transform().is_updated())
    {
        _view_matrix_dirty = true;
    }
}

void camera::set_projection_matrix(glm::mat4 mat)
{
    _projection_matrix = std::move(mat);
}

void camera::set_view_matrix(glm::mat4 mat) { _view_matrix = std::move(mat); }

void camera::render_texture_background()
{
    return;
    // TODO: As we no longer keep the transform, it will be way easier and
    // probably also cheaper to render a sphere instead of a quad

    // auto background_shader =
    //     asset_manager::default_asset_manager()->get_shader("camera_background");
    // _background_texture->set_active_texture(0);
    // background_shader->set_uniform("u_environment_map", 0);
    // background_shader->set_uniform("u_camera_matrix",
    //                                glm::toMat4(get_transform().get_rotation())
    //                                *
    //                                    glm::inverse(projection_matrix()));
    // if (_background_texture)
    // {
    //     _background_texture->set_active_texture(0);
    // }

    // background_shader->use();
    // mesh* quad_mesh =
    // asset_manager::default_asset_manager()->get_mesh("quad");
    // quad_mesh->render();
    // shader_program::unuse();
}

void camera::render_on_private_texture() const
{
    // _framebuffer->bind();
    // // TODO?: maybe better to clear with the specified background color
    // instead
    // // of drawing background quad with that color
    // glClearColor(
    //     _background_color.x, _background_color.y, _background_color.z, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glEnable(GL_DEPTH_TEST);

    // if (scene::get_active_scene())
    // {
    //     scene::get_active_scene()->visit_root_objects(
    //         [](auto& obj)
    //     {
    //         if (!obj->is_active())
    //             return;

    //         if (auto* renderer =
    //                 obj->template try_get<components::mesh_renderer>())
    //         {
    //             auto* mesh =
    //                 obj->template get<components::mesh_filter>().get_mesh();
    //             auto* material = renderer->get_material();
    //             if (material)
    //             {
    //                 material->set_property_value(
    //                     "u_model_matrix", obj->get_transform().get_matrix());
    //                 renderer_3d().draw_mesh(mesh, material);
    //             }
    //         }
    //     });
    // }
    // _framebuffer->unbind();
}

void camera::setup_lights() { }

glm::mat4 camera::calculate_projection_matrix() const
{
    // TODO: optimize with caching
    // copy into floating point vec2
    glm::dvec2 size = _render_size;
    if (_is_orthogonal)
    {
        glm::dquat rotation = get_transform().get_rotation();
        glm::dvec3 direction = rotation * glm::dvec3 { 0, 0, 1 };
        float dist =
            std::abs(glm::dot(direction, get_transform().get_position()));

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
