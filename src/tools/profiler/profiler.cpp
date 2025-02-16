/* clang-format off */
#include <GLFW/glfw3.h>
/* clang-format on */

#include <prof/profiler.hpp>

#include "tools/profiler/profiler.hpp"

#include "assets/asset_manager.hpp"
#include "common/logging.hpp"
#include "graphics/graphics.hpp"
#include "graphics/material.hpp"
#include "graphics/mesh.hpp"
#include "graphics/renderer/renderer_3d.hpp"
#include "graphics/texture.hpp"
#include "graphics/vertex.hpp"

namespace
{
logger log() { return get_logger("profiler"); }
} // namespace

struct profiler::impl
{
    float _zoom { 1 };
    glm::vec2 zoom { .3, .0001 };
    glm::vec2 scroll { 0, 0 };
    bool autoscroll { true };
};

profiler::profiler()
{
    set_title("Profiler");
    resize(600, 600);
    on_user_initialize +=
        [ this ](std::shared_ptr<core::window>) { initialize(); };
}

profiler::~profiler() { }

void profiler::initialize()
{
    _impl = std::make_unique<impl>();
    get_events()->render += [ this ](auto e) { render(); };
    get_events()->resize +=
        [ this ](auto e) { resize(e.get_new_size().x, e.get_new_size().y); };
}

void profiler::render()
{
    graphics::set_viewport({ 0, 0 }, { get_size() });
    graphics::clear({ .1f, .1f, .1f, 1.0f });
    std::stringstream ss;
    ss << std::this_thread::get_id();

    struct prof_data
    {
        float frame_duration;
    };

    std::vector<prof_data> data;

    prof::apply_frames(ss.str(),
                       [ &data,
                         &scroll = _impl->scroll,
                         autoscroll = _impl->autoscroll ](const auto& frame)
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

    auto map_to_window = [ this ](glm::vec2 point,
                                  glm::vec2 window) -> glm::vec2
    { return (point / window - 0.5f) * 2.0f; };

    auto vp_sample_count = get_width() * _impl->zoom.x + 1;

    const auto count = std::min<size_t>(vp_sample_count, data.size());

    if (count < data.size() && _impl->autoscroll)
    {
        _impl->scroll = glm::vec2(data.size() - count, 0) / _impl->zoom;
    }

    for (size_t i = 0; i < count; ++i)
    {
        const auto local_index = data.size() - 1 - i;
        auto& d = data[ local_index ];

        const auto spos = glm::uvec2(local_index, 0);
        const auto pos = glm::vec2(spos) / _impl->zoom;
        const auto lpos = pos - _impl->scroll;
        const auto sample_size = glm::vec2(1, d.frame_duration) / _impl->zoom;

        v[ 0 ].position() = glm::vec3(map_to_window(lpos, get_size()), 0.0f);
        v[ 1 ].position() = glm::vec3(
            map_to_window(lpos + glm::vec2(0, sample_size.y), get_size()),
            0.0f);
        v[ 2 ].position() = glm::vec3(
            map_to_window(lpos + glm::vec2(sample_size.x, 0), get_size()),
            0.0f);
        v[ 3 ].position() =
            glm::vec3(map_to_window(lpos + sample_size, get_size()), 0.0f);
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
