#pragma once

#include "graphics/graphics_fwd.hpp"

#include "graphics/renderer/renderer.hpp"
#include "graphics/vaomap.hpp"

class renderer_3d : public renderer
{
public:
    void draw_mesh(std::shared_ptr<graphics::mesh> m,
                   std::shared_ptr<graphics::material> mat);

private:
    vao_map _vao;
};
