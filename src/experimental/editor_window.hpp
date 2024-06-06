#pragma once

#include "experimental/window.hpp"
#include "utils.hpp"

namespace experimental
{

class editor_window
    : public window
    , public singleton<editor_window>
{
public:
    ~editor_window();

private:
    editor_window();

    void initialize();
    void render_grid();
    void render_axis();

    friend singleton_t;

private:
    glm::vec3 _camera_position;
    glm::vec3 _camera_direction;
    std::optional<glm::vec2> _mouse_position;
};

} // namespace experimental
