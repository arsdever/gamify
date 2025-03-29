#include <assets/asset_manager.hpp>
#include <glad/gl.h>
#include <prof/profiler.hpp>

#include "graphics/renderer/renderer_3d.hpp"

#include "graphics/graphics_buffer.hpp"
#include "graphics/material.hpp"
#include "graphics/mesh.hpp"
#include "graphics/vaomap.hpp"
#include "graphics/vertex.hpp"

void renderer_3d::draw_mesh(std::shared_ptr<graphics::mesh> m,
                            std::shared_ptr<graphics::material> material)
{
    draw_mesh(m, std::vector<std::shared_ptr<graphics::material>> { material });
}

void renderer_3d::draw_mesh(
    std::shared_ptr<graphics::mesh> m,
    std::vector<std::shared_ptr<graphics::material>> materials)
{
    auto sp = prof::profile(__FUNCTION__);

    if (_vao.activate())
    {
        glBindBuffer(GL_ARRAY_BUFFER, m->get_vertex_buffer().get_handle());
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                     m->get_index_buffer().get_handle());
        vertex3d::initialize_attributes();
    }

    vertex3d::activate_attributes();

    if (materials.size() < 2)
    {
        std::shared_ptr<graphics::material> mat;
        if (materials.empty())
        {
            mat = assets::asset_manager::get<graphics::material>(
                "standard.fallback.mat");
        }
        else
        {
            mat = materials[ 0 ];
        }

        mat->activate();

        glDrawElements(GL_TRIANGLES,
                       m->get_index_buffer().get_element_count(),
                       GL_UNSIGNED_INT,
                       0);
    }

    m->apply_submeshes([ & ](graphics::mesh::submesh_info& info)
    {
        auto material_index = info.material_index;
        material_index =
            std::min<unsigned short>(material_index, materials.size() - 1);
        auto mat = materials[ material_index ];

        mat->activate();

        glDrawElements(GL_TRIANGLES,
                       info.index_count,
                       GL_UNSIGNED_INT,
                       (void*)(info.vertex_index_offset * sizeof(int)));
    });
}
