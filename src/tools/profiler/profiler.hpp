#pragma once

#include "core/window.hpp"
#include "prof/data.hpp"

class profiler : public core::window
{
public:
    using element_type =
        std::variant<const prof::frame*, const prof::data_sample*>;

public:
    profiler();
    ~profiler();

    void set_frame(prof::frame f);
    void unset_frame();

    glm::vec2 zoom() const;
    void set_zoom(glm::vec2 z);

    glm::vec2 scroll() const;
    void scroll_to(glm::vec2 s);

    glm::vec2 get_sample_size() const;

    std::optional<element_type> get_at(const glm::uvec2& pos) const;

protected:
    void initialize();
    void render();

    void render_frame(const prof::frame& f);
    void render_overall();

private:
    struct impl;
    std::unique_ptr<impl> _impl;
};
