/* clang-format off */
#include <GLFW/glfw3.h>
/* clang-format on */

#include <prof/profiler.hpp>

#include "tools/profiler/profiler.hpp"

#include "assets/asset_manager.hpp"
#include "common/logging.hpp"
#include "core/algo.hpp"
#include "graphics/graphics.hpp"
#include "graphics/material.hpp"
#include "graphics/mesh.hpp"
#include "graphics/renderer/renderer_3d.hpp"
#include "graphics/texture.hpp"
#include "graphics/vertex.hpp"
#include "profiler_frame_renderer.hpp"
#include "tools/profiler/profiler_renderer.hpp"

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
    auto it = std::find_if(_impl->_presented_elements.begin(),
                           _impl->_presented_elements.end(),
                           [ pos ](const auto& e)
    {
        auto rect = std::get<0>(e);
        return core::rect_contains<double, double, unsigned>({ rect.x, rect.y }, { rect.z, rect.w }, pos);
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

glm::vec2 profiler::zoom() { return _impl->zoom; }

void profiler::set_zoom(glm::vec2 z) { _impl->zoom = std::move(z); }

glm::vec2 profiler::scroll() { return _impl->scroll; }

void profiler::scroll_to(glm::vec2 s) { _impl->scroll = std::move(s); }

void profiler::render()
{
    if (_impl->_frame.has_value())
    {
        profiler_frame_renderer{}.render(*_impl->_frame, get_size());
    }
    else
    {
        profiler_renderer {}.render(get_size(), _impl->zoom, _impl->scroll);
    }
}
