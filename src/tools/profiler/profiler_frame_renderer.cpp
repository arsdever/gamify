#include <assets/asset_manager.hpp>
#include <core/window.hpp>
#include <graphics/framebuffer.hpp>
#include <graphics/graphics.hpp>
#include <graphics/material.hpp>
#include <graphics/mesh.hpp>
#include <graphics/renderer/renderer_2d.hpp>
#include <graphics/renderer/renderer_3d.hpp>
#include <graphics/vertex.hpp>

#include "tools/profiler/profiler_frame_renderer.hpp"

#include "prof/data.hpp"


void profiler_frame_renderer::render(const prof::frame& pf,
                                     const glm::uvec2& viewport_size)
{
    framebuffer::unbind();
    graphics::set_viewport({ 0, 0 }, { viewport_size });
    graphics::clear({ .1f, .1f, .1f, 1.0f });
    auto mouse_pos = core::window::get_active_window()->get_mouse_position();

    size_t layer_count = pf.max_depth() + 1;
    double layer_height = 1.0 / layer_count;

    std::chrono::duration<double> frame_duration = pf.end() - pf.start();

    std::array<vertex3d, 4> sample_vertices;
    // std::vector<std::tuple<std::string, glm::vec2>> texts;
    std::optional<std::tuple<std::string, glm::vec2>> text;
    std::vector<vertex3d> vertices;
    std::vector<int> indices;

    for (auto sample : pf.samples())
    {
        double start_time_relation =
            (sample.start() - pf.start()) / frame_duration;
        double end_time_relation = (sample.end() - pf.start()) / frame_duration;

        auto layer = sample.depth();
        auto layer_pos = layer * layer_height;
        auto layer_size = layer_height;

        glm::dvec4 rect = { start_time_relation,
                            layer_pos,
                            end_time_relation,
                            layer_pos + layer_size };

        sample_vertices[ 0 ].position() =
            (glm::dvec3 { rect.x, rect.y, 0.0 } - glm::dvec3(0.5)) * 2.0;
        sample_vertices[ 1 ].position() =
            (glm::dvec3 { rect.x, rect.w, 0.0 } - glm::dvec3(0.5)) * 2.0;
        sample_vertices[ 2 ].position() =
            (glm::dvec3 { rect.z, rect.w, 0.0 } - glm::dvec3(0.5)) * 2.0;
        sample_vertices[ 3 ].position() =
            (glm::dvec3 { rect.z, rect.y, 0.0 } - glm::dvec3(0.5)) * 2.0;

        indices.insert(indices.end(),
                       { static_cast<int>(vertices.size()) + 0,
                         static_cast<int>(vertices.size()) + 1,
                         static_cast<int>(vertices.size()) + 2,
                         static_cast<int>(vertices.size()) + 2,
                         static_cast<int>(vertices.size()) + 3,
                         static_cast<int>(vertices.size()) + 0 });

        std::chrono::duration<double> sample_duration = sample.diff();
        auto score = sample_duration / frame_duration;
        auto color = glm::vec4(score, 1.0f - score, 0.0f, 1.0f);
        sample_vertices[ 0 ].color() = color / 1.5f;
        sample_vertices[ 1 ].color() = color / 1.5f;
        sample_vertices[ 2 ].color() = color / 1.5f;
        sample_vertices[ 3 ].color() = color / 1.5f;

        vertices.insert(
            vertices.end(), sample_vertices.begin(), sample_vertices.end());

        // check if mouse is in the rect
        auto mp = mouse_pos;
        mp.y = viewport_size.y - mp.y;
        if (mp.x >= rect.x * viewport_size.x &&
            mp.x <= rect.z * viewport_size.x &&
            mp.y >= rect.y * viewport_size.y &&
            mp.y <= rect.w * viewport_size.y)
        {
            // highlight sample
            sample_vertices[ 0 ].color() = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            sample_vertices[ 1 ].color() = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            sample_vertices[ 2 ].color() = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            sample_vertices[ 3 ].color() = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

            // set the text
            text = { sample.name(), mouse_pos };
        }

        // auto txt_pos = glm::vec2 { start_time_relation * viewport_size.x,
        //                            ((layer_count - layer - .5) *
        //                            layer_height) *
        //                                viewport_size.y };
        // texts.emplace_back(sample.name(), txt_pos);
    }

    std::shared_ptr<graphics::mesh> m = std::make_shared<graphics::mesh>();
    m->set_vertices(std::move(vertices));
    m->set_indices(std::move(indices));
    m->init();

    auto mat =
        assets::asset_manager::get<graphics::material>("standard.surface.mat");
    auto txt =
        assets::asset_manager::get<graphics::texture>("images.white.png");

    mat->set_property_value("u_vp_matrix", glm::identity<glm::mat4>());
    mat->set_property_value("u_model_matrix", glm::identity<glm::mat4>());
    mat->set_property_value("u_color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    mat->set_property_value("u_image", txt.get());
    renderer_3d().draw_mesh(m, mat);

    auto font = assets::asset_manager::get<graphics::font>("default.ttf");
    // for (const auto& [ text, pos ] : texts)
    // {
    //     renderer_2d().draw_text(pos, font, viewport_size, text);
    // }
    if (text.has_value())
    {
        auto [ str, pos ] = text.value();
        renderer_2d().draw_text(pos, font, viewport_size, str);
    }
}
