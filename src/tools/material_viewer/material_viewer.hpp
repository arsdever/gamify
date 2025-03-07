#pragma once

#include "graphics/graphics_fwd.hpp"

#include "core/window.hpp"

class material_viewer : public core::window
{
public:
    material_viewer();
    ~material_viewer();

    void set_material(std::shared_ptr<graphics::material> m);
    void set_mesh(std::shared_ptr<graphics::mesh> m);
    void set_background_color(glm::vec4 color);

    glm::mat4 get_camera_matrix() const;
    glm::vec3 get_camera_position() const;

    std::shared_ptr<graphics::material> get_material() const;
    std::shared_ptr<graphics::mesh> get_mesh() const;

private:
    void initialize();
    void render();

private:
    std::shared_ptr<graphics::material> _material { nullptr };
    std::shared_ptr<graphics::mesh> _mesh;
    std::unique_ptr<graphics_buffer> _light_buffer { nullptr };
    glm::vec2 _rotation { 0.0 };
    glm::vec2 _camera_rotation { 0.0 };
    glm::vec2 _rotation_start_point { 0.0 };
    glm::vec2 _camera_rotation_start_point { 0.0 };
    glm::vec4 _background_color { 0.0, 0.0, 0.0, 1.0 };
    double _zoom { 1.0 };
    bool _draw_wireframe { false };
};
