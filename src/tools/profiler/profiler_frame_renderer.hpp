#pragma once

#include "prof/data.hpp"
class profiler_frame_renderer
{
public:
    profiler_frame_renderer() = default;

    void render(const prof::frame& pf, const glm::uvec2& viewport_size);
};
