#pragma once

#include "renderer.hpp"
#include "vaomap.hpp"

class material;
class mesh;

class renderer_3d : public renderer
{
public:
    void draw_mesh(mesh* m, material* mat);

    static void draw_grid(float grid_size,
                          const glm::mat4& model,
                          const glm::mat4& view,
                          const glm::mat4& proj);
    static void render_transform_controls(transform& t,
                                          const glm::mat4& view,
                                          const glm::mat4& proj);
    static void draw_ray(const glm::vec3& origin,
                         const glm::vec3& direction,
                         float length,
                         const glm::vec4& color,
                         const glm::mat4& view,
                         const glm::mat4& proj);

    static renderer_3d& instance();

private:
    vao_map _vao;
    static std::unique_ptr<renderer_3d> _instance;
};
