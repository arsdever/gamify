#pragma once

#include <nlohmann/json_fwd.hpp>

#include "graphics/graphics_fwd.hpp"
#include "project/project_fwd.hpp"

#include "project/component_interface/component.hpp"

namespace components
{
class camera : public component
{
public:
    using base = component;

    camera(game_object& obj);
    camera& operator=(camera&& obj);
    ~camera() override;

    void set_fov(double fov);
    double get_fov() const;

    void set_orthogonal(bool ortho_flag = true);
    bool is_orthogonal() const;

    double get_aspect_ratio() const;

    void set_active();
    static camera* get_active();
    static std::vector<camera*> all();

    void set_render_size(size_t width, size_t height);
    void set_render_size(glm::uvec2 size);
    void get_render_size(size_t& width, size_t& height) const;
    glm::uvec2 get_render_size() const;

    void set_render_texture(std::weak_ptr<graphics::texture> render_texture);
    std::shared_ptr<graphics::texture> get_render_texture() const;

    void set_background_color(glm::dvec4 color);
    glm::dvec4 get_background_color() const;

    void render();

    glm::mat4 projection_matrix() const;
    glm::mat4 view_matrix() const;
    glm::mat4 vp_matrix() const;

    static constexpr std::string_view type_name = "camera";

    template <typename ST>
    void serialize(ST& s);
    void deserialize(const nlohmann::json& j);

protected:
    void on_init() override;
    void on_update() override;

private:
    void set_projection_matrix(glm::mat4 mat);
    void set_view_matrix(glm::mat4 mat);

    void render_texture_background();
    void render_on_private_texture() const;
    void setup_lights();

    glm::mat4 calculate_projection_matrix() const;

private:
    glm::uvec2 _render_size { 1u };
    double _field_of_view { .6 };
    bool _is_orthogonal { false };
    std::weak_ptr<graphics::texture> _user_render_texture {};
    glm::dvec4 _background_color { 0.0 };
    std::unique_ptr<graphics_buffer> _lights_buffer {};

    glm::dmat4 _view_matrix;
    glm::dmat4 _projection_matrix;
    bool _projection_matrix_dirty { false };

    static camera* _active_camera;
    static std::vector<camera*> _cameras;
    std::unique_ptr<framebuffer> _framebuffer { nullptr };
};
} // namespace components
