#pragma once

class profiler_renderer
{
public:
    void render(const glm::uvec2& size,
                const glm::vec2& zoom,
                const glm::vec2& scroll);
};
