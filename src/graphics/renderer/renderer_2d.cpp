#include "renderer_2d.hpp"

#include "assets/asset_manager.hpp"
#include "glad/gl.h"
#include "graphics/font.hpp"
#include "graphics/graphics_buffer.hpp"
#include "graphics/material.hpp"
#include "graphics/mesh.hpp"
#include "graphics/renderer/algorithms/polygon_to_mesh.hpp"
#include "graphics/texture.hpp"
#include "graphics/vaomap.hpp"
#include "graphics/vertex.hpp"

template <typename T>
glm::tvec2<T> map_from_window(const glm::tvec2<T>& p,
                              const glm::uvec2& window_size)
{
    return glm::tvec2<T>(p.x / static_cast<T>(window_size.x),
                         (window_size.y - p.y) /
                             static_cast<T>(window_size.y)) *
               static_cast<T>(2) -
           glm::tvec2<T>(1);
}

void renderer_2d::draw_rect(glm::vec2 top_left,
                            glm::vec2 bottom_right,
                            glm::vec2 window_size,
                            std::shared_ptr<graphics::texture> txt)
{
    glm::vec2 bottom_left { top_left.x, bottom_right.y };
    glm::vec2 top_right { bottom_right.x, top_left.y };
    std::array<vertex3d, 4> vertices {};
    vertices[ 0 ].position() = { map_from_window<float>(top_left, window_size),
                                 0.0f };
    vertices[ 1 ].position() = {
        map_from_window<float>(bottom_left, window_size), 0.0f
    };
    vertices[ 2 ].position() = {
        map_from_window<float>(bottom_right, window_size), 0.0f
    };
    vertices[ 3 ].position() = { map_from_window<float>(top_right, window_size),
                                 0.0f };
    vertices[ 0 ].uv() = { 0, 1 };
    vertices[ 1 ].uv() = { 0, 0 };
    vertices[ 2 ].uv() = { 1, 0 };
    vertices[ 3 ].uv() = { 1, 1 };

    std::array<unsigned, 6> indices { 0, 1, 2, 0, 2, 3 };

    vao_map vao;
    graphics_buffer vbo(graphics_buffer::type::vertex);
    graphics_buffer ebo(graphics_buffer::type::index);

    vbo.set_element_stride(vertex3d::size);
    vbo.set_element_count(vertices.size());
    ebo.set_element_stride(sizeof(unsigned));
    ebo.set_element_count(indices.size());
    vbo.set_data(vertices.data());
    ebo.set_data(indices.data());

    if (vao.activate())
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo.get_handle());
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.get_handle());
        vertex3d::initialize_attributes();
    }

    glDisable(GL_DEPTH_TEST);
    vertex3d::activate_attributes();

    auto shader_2d =
        assets::asset_manager::get<graphics::material>("standard.surface.mat");
    shader_2d->set_property_value("u_vp_matrix", glm::identity<glm::mat4>());
    shader_2d->set_property_value("u_model_matrix", glm::identity<glm::mat4>());

    shader_2d->set_property_value("u_color", glm::vec4(1));
    shader_2d->set_property_value("u_image", txt);
    shader_2d->activate();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glEnable(GL_DEPTH_TEST);
}

void renderer_2d::draw_rect(glm::vec2 top_left,
                            glm::vec2 bottom_right,
                            glm::vec2 window_size,
                            float border_thickness,
                            glm::vec4 border_color,
                            glm::vec4 fill_color)
{
    std::vector<glm::vec2> points;
    std::vector<vertex3d> vertices;
    std::vector<unsigned> indices;

    glm::vec2 bottom_left { top_left.x, bottom_right.y };
    glm::vec2 top_right { bottom_right.x, top_left.y };
    points = {
        top_left,
        bottom_left,
        bottom_right,
        top_right,
    };

    for (const auto& point : points)
    {
        vertex3d v;
        v.position() =
            glm::vec3(map_from_window<float>(point, window_size), 0.0f);
        v.uv() = map_from_window<float>(point, window_size);
        vertices.push_back(v);
    }

    indices = { 0, 1, 2, 0, 2, 3 };
    unsigned index_offset = vertices.size();

    if (border_thickness > 0.0f)
    {
        polygon_to_mesh(points,
                        true,
                        border_thickness,
                        [ border_color, window_size, &vertices ](glm::vec2 v)
        {
            vertex3d vert;
            vert.position() =
                glm::vec3(map_from_window<float>(v, window_size), 0.0f);
            vert.uv() = map_from_window<float>(v, window_size);
            vertices.push_back(vert);
        },
                        [ index_offset, &indices ](unsigned i)
        { indices.push_back(i + index_offset); });
    }

    vao_map vao;
    graphics_buffer vbo(graphics_buffer::type::vertex);
    graphics_buffer ebo(graphics_buffer::type::index);

    vbo.set_element_stride(vertex3d::size);
    vbo.set_element_count(vertices.size());
    ebo.set_element_stride(sizeof(unsigned));
    ebo.set_element_count(indices.size());
    vbo.set_data(vertices.data());
    ebo.set_data(indices.data());

    if (vao.activate())
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo.get_handle());
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.get_handle());
        vertex3d::initialize_attributes();
    }

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vertex3d::activate_attributes();

    auto shader_2d =
        assets::asset_manager::get<graphics::material>("standard.surface.mat");
    shader_2d->set_property_value("u_vp_matrix", glm::identity<glm::mat4>());
    shader_2d->set_property_value("u_model_matrix", glm::identity<glm::mat4>());

    glm::uvec2 usize = window_size;
    shader_2d->set_property_value("u_color", fill_color);
    shader_2d->activate();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    if (border_thickness > 0.0f)
    {
        shader_2d->set_property_value("u_color", border_color);
        shader_2d->activate();
        glDrawElements(GL_TRIANGLES,
                       indices.size() - 6,
                       GL_UNSIGNED_INT,
                       (void*)(6 * sizeof(unsigned)));
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void renderer_2d::draw_text(glm::vec2 baseline,
                            std::shared_ptr<graphics::font> f,
                            const glm::vec2& window_size,
                            std::string_view text,
                            glm::vec4 color,
                            float fscale)
{
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // iterate through all characters
    std::string::const_iterator c;
    // while rendering local to world conversion is already considered
    glm::vec2 scale = glm::vec2(fscale);
    glm::vec2 cursor_position = baseline;

    std::vector<vertex3d> vertices;
    std::vector<int> indices;

    // tl, tr, br, bl
    std::array<vertex3d, 4> quad_vertices {};

    for (const auto& c : text)
    {
        const graphics::font::character& ch = (*f)[ static_cast<size_t>(c) ];

        float xpos = cursor_position.x + ch._bearing.x * scale.x;
        float ypos = cursor_position.y - ch._bearing.y * scale.y;

        float w = ch._size.x * scale.x;
        float h = ch._size.y * scale.y;

        quad_vertices[ 0 ].position() = {
            map_from_window<float>(glm::vec2 { xpos, ypos + h }, window_size),
            0.0f
        };
        quad_vertices[ 1 ].position() = {
            map_from_window<float>(glm::vec2 { xpos, ypos }, window_size), 0.0f
        };
        quad_vertices[ 2 ].position() = { map_from_window<float>(
                                              glm::vec2 { xpos + w, ypos + h },
                                              window_size),
                                          0.0f };
        quad_vertices[ 3 ].position() = {
            map_from_window<float>(glm::vec2 { xpos + w, ypos }, window_size),
            0.0f
        };

        quad_vertices[ 0 ].uv() = glm::vec2 {
            ch._texture_offset.x, ch._texture_offset.y + ch._size.y
        } / static_cast<glm::vec2>(f->atlas()->get_size());
        quad_vertices[ 1 ].uv() =
            glm::vec2 { ch._texture_offset.x, ch._texture_offset.y } /
            static_cast<glm::vec2>(f->atlas()->get_size());
        quad_vertices[ 2 ].uv() = glm::vec2 {
            ch._texture_offset.x + ch._size.x, ch._texture_offset.y + ch._size.y
        } / static_cast<glm::vec2>(f->atlas()->get_size());
        quad_vertices[ 3 ].uv() = glm::vec2 {
            ch._texture_offset.x + ch._size.x, ch._texture_offset.y
        } / static_cast<glm::vec2>(f->atlas()->get_size());

        const auto n = vertices.size();

        for (int i = 0; i < 4; i++)
        {
            vertices.push_back(quad_vertices[ i ]);
            indices.push_back(n);
            indices.push_back(n + 1);
            indices.push_back(n + 2);
            indices.push_back(n + 2);
            indices.push_back(n + 1);
            indices.push_back(n + 3);
        }

        // now advance cursors for next glyph (note that advance is number
        // of 1/64 pixels)
        cursor_position.x +=
            ch._advance / 64.0f *
            scale.x; // bitshift by 6 to get value in pixels (2^6 = 64)
    }

    graphics::mesh text_mesh;
    text_mesh.set_vertices(std::move(vertices));
    text_mesh.set_indices(std::move(indices));
    text_mesh.init();

    auto white =
        assets::asset_manager::get<graphics::texture>("images.white.png");

    auto text_material =
        assets::asset_manager::get<graphics::material>("standard.surface.mat");
    text_material->set_property_value("u_vp_matrix",
                                      glm::identity<glm::mat4>());
    text_material->set_property_value("u_model_matrix",
                                      glm::identity<glm::mat4>());

    text_material->set_property_value("u_color", color);
    text_material->set_property_value("u_mask", f->atlas());
    text_material->set_property_value("u_image", white);
    text_material->activate();
    text_mesh.render();
    text_material->set_property_value("u_mask", white);

#ifdef RENDER_TEXT_CHAR_BORDER
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    surface_shader->set_property_value("u_color", color);
    surface_shader->set_property_value("u_image", white);
    surface_shader->activate();
    text_mesh.render();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}
