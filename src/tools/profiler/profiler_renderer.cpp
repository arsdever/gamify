#include <assets/asset_manager.hpp>
#include <graphics/framebuffer.hpp>
#include <graphics/graphics.hpp>
#include <graphics/material.hpp>
#include <graphics/mesh.hpp>
#include <graphics/renderer/renderer_3d.hpp>
#include <prof/profiler.hpp>

#include "tools/profiler/profiler_renderer.hpp"

void profiler_renderer::render(const glm::uvec2& size,
                               const glm::vec2& zoom,
                               const glm::vec2& scroll)
{
    framebuffer::unbind();
    graphics::set_viewport({ 0, 0 }, { size });
    graphics::clear({ .1f, .1f, .1f, 1.0f });
    auto sc = scroll;

    std::stringstream ss;
    ss << std::this_thread::get_id();

    struct prof_data
    {
        float frame_duration;
    };

    std::vector<prof_data> data;

    prof::apply_frames(ss.str(),
                       [ &data ](const auto& frame)
    {
        data.emplace_back(
            std::chrono::duration_cast<std::chrono::duration<float>>(
                frame.end() - frame.start())
                .count());
        return true;
    });

    std::vector<vertex3d> vertices;
    std::vector<int> indices;
    std::array<vertex3d, 4> v;

    auto map_to_window = [](glm::vec2 point, glm::vec2 window) -> glm::vec2
    { return (point / window - 0.5f) * 2.0f; };

    auto vp_sample_count = size.x * zoom.x + 1;

    const auto count = std::min<size_t>(vp_sample_count, data.size());

    if (count < data.size())
    {
        sc = glm::vec2(data.size() - count, 0) / zoom;
    }

    for (size_t i = 0; i < count; ++i)
    {
        const auto local_index = data.size() - 1 - i;
        auto& d = data[ local_index ];

        const auto spos = glm::uvec2(local_index, 0);
        const auto pos = glm::vec2(spos) / zoom;
        const auto lpos = pos - sc;
        const auto sample_size = glm::vec2(1, d.frame_duration) / zoom;

        v[ 0 ].position() = glm::vec3(map_to_window(lpos, size), 0.0f);
        v[ 1 ].position() = glm::vec3(
            map_to_window(lpos + glm::vec2(0, sample_size.y), size), 0.0f);
        v[ 2 ].position() = glm::vec3(
            map_to_window(lpos + glm::vec2(sample_size.x, 0), size), 0.0f);
        v[ 3 ].position() =
            glm::vec3(map_to_window(lpos + sample_size, size), 0.0f);
        indices.insert(indices.end(),
                       { static_cast<int>(vertices.size()) + 0,
                         static_cast<int>(vertices.size()) + 1,
                         static_cast<int>(vertices.size()) + 2,
                         static_cast<int>(vertices.size()) + 2,
                         static_cast<int>(vertices.size()) + 1,
                         static_cast<int>(vertices.size()) + 3 });
        vertices.insert(vertices.end(), v.begin(), v.end());
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
}
