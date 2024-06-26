#pragma once

#include <memory>
#include <string_view>

#include <glm/fwd.hpp>

class camera;

namespace experimental
{

class window;

/**
 * @brief Class representing a viewport inside the window.
 *
 * Viewport is a section within the window (may be the whole window area too)
 * and renders some part of the scene.
 */
class viewport
{
private:
    struct viewport_private;

public:
    viewport();
    ~viewport();

    void initialize();

    void set_position(glm::vec2 pos);
    glm::vec2 get_position() const;

    void set_size(glm::vec2 size);
    glm::vec2 get_size() const;

    void set_camera(std::weak_ptr<camera> cam);
    std::shared_ptr<camera> get_camera() const;

    void render();
    void take_screenshot(std::string_view path);

    static viewport* current_viewport();

private:
    std::unique_ptr<viewport_private> _p;
    static viewport* _current_viewport;
};

} // namespace experimental
