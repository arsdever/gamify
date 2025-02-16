#pragma once

#include "graphics/graphics_fwd.hpp"

#include "core/window.hpp"

class texture_viewer : public core::window
{
public:
    texture_viewer();
    ~texture_viewer();

    void set_texture(std::shared_ptr<graphics::texture> m);

private:
    void initialize();
    void render();

private:
    std::shared_ptr<graphics::texture> _texture { nullptr };
    glm::vec2 _offset { 0 };
    glm::vec2 _pan_start { 0 };
    bool _panning { false };
    double _zoom { 1.0 };
};
