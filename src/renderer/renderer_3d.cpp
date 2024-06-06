#include <prof/profiler.hpp>

#include "renderer_3d.hpp"

#include "asset_manager.hpp"
#include "camera.hpp"
#include "experimental/viewport.hpp"
#include "glad/gl.h"
#include "graphics_buffer.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "transform.hpp"
#include "vaomap.hpp"
#include "vertex.hpp"

void renderer_3d::draw_mesh(mesh* m, material* mat)
{
    auto sp = prof::profile(__FUNCTION__);
    if (_vao.activate())
    {
        glBindBuffer(GL_ARRAY_BUFFER, m->get_vertex_buffer().get_handle());
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                     m->get_index_buffer().get_handle());
        vertex3d::initialize_attributes();
    }

    vertex3d::activate_attributes();

    if (auto cam = camera::active_camera())
    {
        mat->set_property_value("u_vp_matrix", cam->vp_matrix());
        mat->set_property_value("u_camera_position",
                                cam->get_transform().get_position());
    }

    mat->activate();

    glDrawElements(GL_TRIANGLES,
                   m->get_index_buffer().get_element_count(),
                   GL_UNSIGNED_INT,
                   0);
}

void renderer_3d::draw_grid(float grid_size,
                            const glm::mat4& model,
                            const glm::mat4& view,
                            const glm::mat4& proj)
{
    auto sp = prof::profile(__FUNCTION__);
    std::array<simple_vertex2d, 2> line;
    line[ 0 ].position() = glm::vec2 { 0, -1 };
    line[ 1 ].position() = glm::vec2 { 0, 1 };
    std::array<unsigned, 2> indices = { 0, 1 };

    graphics_buffer vbo(graphics_buffer::type::vertex);
    graphics_buffer ebo(graphics_buffer::type::index);

    vbo.set_element_count(2);
    vbo.set_element_stride(simple_vertex2d::size);
    vbo.set_data(line.data());

    ebo.set_element_count(2);
    ebo.set_element_stride(sizeof(unsigned));
    ebo.set_data(indices.data());

    // TODO: crashes when changing to instance()._vao.activate()
    vao_map vao;
    if (vao.activate())
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo.get_handle());
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.get_handle());

        simple_vertex2d::initialize_attributes();
    }

    simple_vertex2d::activate_attributes();

    constexpr int grid_count = 82;

    auto grid_shader =
        asset_manager::default_asset_manager()->get_shader("grid");
    grid_shader->set_uniform("u_grid_count", grid_count);
    grid_shader->set_uniform("u_grid_size", grid_size / 2.0f);
    grid_shader->set_uniform("u_model_matrix", model);
    grid_shader->set_uniform("u_vp_matrix", proj * view);

    grid_shader->use();
    glDrawElementsInstanced(GL_LINES, 2, GL_UNSIGNED_INT, nullptr, grid_count);
}

void renderer_3d::render_transform_controls(transform& t,
                                            const glm::mat4& view,
                                            const glm::mat4& proj)
{
    auto sp = prof::profile(__FUNCTION__);

    auto arrow_mesh = asset_manager::default_asset_manager()->get_mesh(
        "translation_arrow_mesh");
    auto shader =
        asset_manager::default_asset_manager()->get_shader("basic_lit");

    vao_map vao;
    if (vao.activate())
    {
        glBindBuffer(GL_ARRAY_BUFFER,
                     arrow_mesh->get_vertex_buffer().get_handle());
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                     arrow_mesh->get_index_buffer().get_handle());

        vertex3d::initialize_attributes();
    }
    vertex3d::activate_attributes();

    shader->set_uniform("u_vp_matrix", proj * view);
    shader->set_uniform("u_model_matrix", glm::identity<glm::mat4>());
    shader->set_uniform("u_color", glm::vec4 { 0.82, 0.33, 0.35, 1.0 });
    shader->use();
    glDrawElements(GL_TRIANGLES,
                   arrow_mesh->get_index_buffer().get_element_count(),
                   GL_UNSIGNED_INT,
                   nullptr);
    shader->set_uniform("u_color", glm::vec4 { 0.46, 0.61, 0.11, 1.0 });
    shader->set_uniform("u_model_matrix",
                        glm::rotate(glm::identity<glm::mat4>(),
                                    glm::radians(90.0f),
                                    glm::vec3 { 0, 0, 1 }));
    shader->use();
    glDrawElements(GL_TRIANGLES,
                   arrow_mesh->get_index_buffer().get_element_count(),
                   GL_UNSIGNED_INT,
                   nullptr);
    shader->set_uniform("u_color", glm::vec4 { 0.2, 0.42, 0.64, 1.0 });
    shader->set_uniform("u_model_matrix",
                        glm::rotate(glm::identity<glm::mat4>(),
                                    glm::radians(-90.0f),
                                    glm::vec3 { 0, 1, 0 }));
    shader->use();
    glDrawElements(GL_TRIANGLES,
                   arrow_mesh->get_index_buffer().get_element_count(),
                   GL_UNSIGNED_INT,
                   nullptr);
}

void renderer_3d::draw_ray(const glm::vec3& origin,
                           const glm::vec3& direction,
                           float length,
                           const glm::vec4& color,
                           const glm::mat4& view,
                           const glm::mat4& proj)
{
    auto sp = prof::profile(__FUNCTION__);
    std::array<simple_vertex3d, 2> line;
    line[ 0 ].position() = origin;
    line[ 1 ].position() = origin + glm::normalize(direction) * length;
    std::array<unsigned, 2> indices = { 0, 1 };

    graphics_buffer vbo(graphics_buffer::type::vertex);
    graphics_buffer ebo(graphics_buffer::type::index);

    vbo.set_element_count(2);
    vbo.set_element_stride(simple_vertex3d::size);
    vbo.set_data(line.data());

    ebo.set_element_count(2);
    ebo.set_element_stride(sizeof(unsigned));
    ebo.set_data(indices.data());

    // TODO: crashes when changing to instance()._vao.activate()
    vao_map vao;
    if (vao.activate())
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo.get_handle());
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.get_handle());

        simple_vertex3d::initialize_attributes();
    }

    simple_vertex3d::activate_attributes();

    auto line_shader =
        asset_manager::default_asset_manager()->get_shader("line");
    line_shader->set_uniform("u_color", color);
    line_shader->set_uniform("u_model_matrix", glm::identity<glm::mat4>());
    line_shader->set_uniform("u_vp_matrix", proj * view);

    line_shader->use();
    glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, nullptr);
}

renderer_3d& renderer_3d::instance()
{
    if (!_instance)
    {
        _instance = std::make_unique<renderer_3d>();
    }
    return *_instance;
}

std::unique_ptr<renderer_3d> renderer_3d::_instance = nullptr;
