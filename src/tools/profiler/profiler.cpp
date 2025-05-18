/* clang-format off */
#include <string>
#include <GLFW/glfw3.h>
/* clang-format on */

#include "tools/profiler/profiler.hpp"

#include "assets/asset_manager.hpp"
#include "common/logging.hpp"
#include "core/algo.hpp"
#include "graphics/framebuffer.hpp"
#include "graphics/graphics.hpp"
#include "graphics/material.hpp"
#include "graphics/mesh.hpp"
#include "graphics/renderer/renderer_3d.hpp"
#include "graphics/texture.hpp"
#include "graphics/vertex.hpp"
#include "profiler_frame_renderer.hpp"

struct profiler::impl
{
    float _zoom { 1 };
    glm::vec2 zoom { 1, 1 };
    glm::vec2 scroll { 0, 0 };
    bool autoscroll { true };

    std::vector<std::tuple<glm::dvec4, element_type>> _presented_elements;
    std::optional<prof::frame> _frame;
};

profiler::profiler()
{
    set_title("Profiler");
    resize(600, 600);
    on_user_initialize +=
        [ this ](std::shared_ptr<core::window>) { initialize(); };
}

profiler::~profiler() { }

std::optional<profiler::element_type>
profiler::get_at(const glm::uvec2& pos) const
{
    auto transformed_pos = pos;
    transformed_pos.y = get_height() - transformed_pos.y;
    auto it = std::find_if(_impl->_presented_elements.begin(),
                           _impl->_presented_elements.end(),
                           [ pos = std::move(transformed_pos) ](const auto& e)
    {
        auto rect = std::get<0>(e);
        return core::rect_contains<double, double, unsigned>(
            { rect.x, rect.y }, { rect.z, rect.w }, pos);
    });

    if (it == _impl->_presented_elements.end())
    {
        return {};
    }

    return std::get<1>(*it);
}

void profiler::initialize()
{
    _impl = std::make_unique<impl>();
    get_events()->render += [ this ](auto e) { render(); };
    get_events()->resize +=
        [ this ](auto e) { resize(e.get_new_size().x, e.get_new_size().y); };
}

void profiler::set_frame(prof::frame f) { _impl->_frame = std::move(f); }

void profiler::unset_frame() { _impl->_frame = {}; }

glm::vec2 profiler::zoom() const { return _impl->zoom; }

void profiler::set_zoom(glm::vec2 z) { _impl->zoom = std::move(z); }

glm::vec2 profiler::scroll() const { return _impl->scroll; }

void profiler::scroll_to(glm::vec2 s)
{
    _impl->scroll = std::move(s);
    std::stringstream ss;
    ss << std::this_thread::get_id();
    auto max_scroll =
        (prof::available_frames_count(ss.str()) - 1) * get_sample_size().x -
        get_size().x;
    if (_impl->scroll.x > max_scroll)
    {
        _impl->scroll.x = max_scroll;
    }
    if (_impl->scroll.x < 0)
    {
        _impl->scroll.x = 0;
    }
}

void profiler::render()
{
    if (_impl->_frame.has_value())
    {
        render_frame(_impl->_frame.value());
    }
    else
    {
        render_overall();
    }
}

glm::vec2 profiler::get_sample_size() const
{
    // 5px per sample
    static constexpr float default_horizontal_sample_width = 5.0f;

    // 10px per 1millisecond
    static constexpr float vertical_pixels_per_ms = 10.0f;

    glm::vec2 sample_size { default_horizontal_sample_width,
                            vertical_pixels_per_ms };

    return sample_size * zoom();
}

void profiler::render_frame(const prof::frame& f)
{
    framebuffer::unbind();
    graphics::set_viewport({ 0, 0 }, { get_size() });
    graphics::clear({ .1f, .1f, .1f, 1.0f });

    if (core::window::get_active_window() == nullptr)
    {
        return;
    }

    auto mouse_pos = core::window::get_active_window()->get_mouse_position();

    auto size = get_size();
    size_t layer_count = f.max_depth() + 1;
    double layer_height = 1.0 / layer_count;

    std::chrono::duration<double> frame_duration = f.end() - f.start();

    std::array<vertex3d, 4> sample_vertices;
    // std::vector<std::tuple<std::string, glm::vec2>> texts;
    std::optional<std::tuple<std::string, glm::vec2>> text;
    std::vector<vertex3d> vertices;
    std::vector<int> indices;
    _impl->_presented_elements.clear();

    for (auto& sample : f.samples())
    {
        double start_time_relation =
            (sample.start() - f.start()) / frame_duration;
        double end_time_relation = (sample.end() - f.start()) / frame_duration;

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
        mp.y = size.y - mp.y;
        if (mp.x >= rect.x * size.x && mp.x <= rect.z * size.x &&
            mp.y >= rect.y * size.y && mp.y <= rect.w * size.y)
        {
            // highlight sample
            sample_vertices[ 0 ].color() = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            sample_vertices[ 1 ].color() = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            sample_vertices[ 2 ].color() = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            sample_vertices[ 3 ].color() = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

            // set the text
            text = { sample.name(), mouse_pos };
        }

        _impl->_presented_elements.push_back(
            { rect * glm::dvec4(size, size), &sample });

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
}

void profiler::render_overall()
{
    auto size = get_size();
    auto zoom = _impl->zoom;
    auto scroll = _impl->scroll;

    // Expect each frame to be less than 100ms by default
    // This matches the worst fps expected - 10 fps
    static constexpr float default_frame_duration = 1.0f / 10.0f;
    // This is also the default zoom on the y axis, meaning 100ms should be
    // filling the size.y
    static constexpr float vertical_pixels_per_ms = 1.0f / 1000.0f;

    // Expect the window to visualize 100 frames by default
    static constexpr float default_frame_count = 100.0f;

    // The above parameters can be varied with zoom and scale options

    framebuffer::unbind();
    graphics::set_viewport({ 0, 0 }, { size });

    // TODO: This can be configurable
    graphics::clear({ .1f, .1f, .1f, 1.0f });

    std::vector<vertex3d> vertices;
    std::vector<int> indices;
    std::array<vertex3d, 4> v;

    auto map_screen_to_gl = [](glm::vec2 point, glm::vec2 window) -> glm::vec2
    { return (point / window - 0.5f) * 2.0f; };

    glm::vec2 sample_size = get_sample_size();

    // Find indices that should be rendered
    // TODO: Implement scrolling, currently hardcoded to 0
    // Expecting the scroll in pixels
    size_t start_index = scroll.x / sample_size.x;
    size_t end_index = ceil(size.x / sample_size.x) + start_index + 1;

    // TODO: The tread id should be configurable
    std::stringstream ss;
    ss << std::this_thread::get_id();

    // TODO: Use the frames vector instead?
    struct prof_data
    {
        float frame_duration;
    };

    std::vector<const prof::frame*> frames;
    size_t index = 0;

    frames.reserve(prof::available_frames_count(ss.str()));
    prof::apply_frames(
        ss.str(),
        [ &frames, &index, start_index, end_index ](const auto& frame)
    {
        while (index++ < start_index)
        {
            return true;
        }

        if (index > end_index)
        {
            return false;
        }

        frames.emplace_back(&frame);
        return true;
    });
    end_index = std::min(end_index, frames.size());

    _impl->_presented_elements.clear();

    auto mouse_pos = get_mouse_position();
    auto it = frames.begin();
    for (size_t i = 0; i < frames.size(); ++i)
    {
        const auto frame_index = i + start_index;
        auto& frame = *frames[ i ];
        float frame_duration =
            std::chrono::duration_cast<std::chrono::duration<double>>(
                frame.end() - frame.start())
                .count();

        const auto lb =
            glm::vec2(static_cast<float>(frame_index), 0) * sample_size -
            scroll;
        const auto ru =
            lb + glm::vec2(1.0f, frame_duration * 1000.0f) * sample_size -
            glm::vec2(1.0f, 0.0f);

        _impl->_presented_elements.push_back(
            { glm::dvec4 { lb, ru }, frames[ i ] });

        v[ 0 ].position() =
            glm::vec3(map_screen_to_gl({ lb.x, ru.y }, size), 0.0f);
        v[ 1 ].position() = glm::vec3(map_screen_to_gl(ru, size), 0.0f);
        v[ 2 ].position() = glm::vec3(map_screen_to_gl(lb, size), 0.0f);
        v[ 3 ].position() =
            glm::vec3(map_screen_to_gl({ ru.x, lb.y }, size), 0.0f);

        // Calculate the vertex color based on the frame duration
        // - if the frame is no longer than 1/60 second, it's green
        // - if the frame is about 1/30 second, it's yellow
        // - if the frame is about 1/10 second or longer, it's red
        float percent = (frame_duration - 1.0f / 60.0f) / (1.0f / 30.0f);
        if (percent < 0.0f)
        {
            // faster than 60 fps
            percent = 0.0f;
        }

        if (percent > 1.0f)
        {
            // slower than 30 fps
            percent /= 3.0f;
            // is it slower than 10 fps?
            if (percent > 1.0f)
            {
                percent = 1.0f;
            }
            else
            {
                percent = 0.5f + percent / 2.0f;
            }
        }
        else
        {
            percent = percent / 2.0f;
        }

        glm::vec4 color = { percent, 1.0f - percent, 0.0f, 1.0f };

        auto mpos = mouse_pos;
        mpos.y = size.y - mpos.y;
        if (core::rect_contains(lb, ru, mpos))
        {
            color = { percent * .5f, (1.0f - percent) * .5f, .7f, 1.0f };
        }

        v[ 0 ].color() = color;
        v[ 1 ].color() = color;
        v[ 2 ].color() = color;
        v[ 3 ].color() = color;

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
