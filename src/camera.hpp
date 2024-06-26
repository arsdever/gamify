#pragma once

#include "graphics_buffer.hpp"
#include "transform.hpp"

class framebuffer;
class image;
class texture;
class mesh;
class shader_program;

class camera
{
public:
    camera();
    ~camera();

    /**
     * @brief Set the field of view (horizontal) of the camera in radians
     *
     * @param fov horizontal field of view in radians
     */
    void set_fov(float fov);

    /**
     * @brief Get the field of view angle value
     *
     * @return float the value of field of view in radians
     */
    float get_fov() const;
    void set_ortho(bool ortho_flag = true);
    float get_aspect_ratio() const;
    camera* set_active();
    void set_render_size(size_t width, size_t height);
    void set_render_size(glm::uvec2 size);
    void set_render_texture(std::weak_ptr<texture> render_texture);
    std::shared_ptr<texture> get_render_texture() const;
    void set_gizmos_enabled(bool flag = true);
    bool get_gizmos_enabled() const;

    void set_background(glm::vec3 color);
    void set_background(image* img);

    void render();

    transform& get_transform();
    const transform& get_transform() const;

    static camera* active_camera();
    static const std::vector<camera*>& all_cameras();

    glm::mat4 projection_matrix() const;
    glm::mat4 view_matrix() const;
    glm::mat4 vp_matrix() const;

private:
    void render_texture_background();
    void render_on_private_texture() const;
    void render_gizmos() const;
    void setup_lights();

private:
    transform _transformation;
    glm::uvec2 _render_size { 1, 1 };
    float _fov = .6f;
    bool _ortho_flag = false;
    std::weak_ptr<texture> _user_render_texture {};
    glm::vec3 _background_color { 0.0f, 0.0f, 0.0f };
    std::unique_ptr<texture> _background_texture = nullptr;
    graphics_buffer _lights_buffer { graphics_buffer::type::shader_storage };
    bool _gizmos_enabled = false;
    std::unique_ptr<framebuffer> _framebuffer { nullptr };

    static camera* _active_camera;
    static std::vector<camera*> _cameras;
};
