#pragma once

#include "graphics/graphics_buffer.hpp"
#include "graphics/vaomap.hpp"
#include "graphics/vertex.hpp"

namespace graphics
{
class mesh
{
public:
    struct submesh_info
    {
        size_t vertex_index_offset;
        size_t vertex_count;
        unsigned short material_index;
    };

public:
    void init();

    void set_vertices(std::vector<vertex3d> positions);
    void set_indices(std::vector<int> indices);
    void recalculate_normals();
    void set_submeshes(std::vector<submesh_info> submeshes);

    // TODO: not the best approach
    // SUGGESTION: move the logic into the renderer class. The last will also
    // manage the vao creation per context
    void render();

    const graphics_buffer& get_vertex_buffer() const;
    const graphics_buffer& get_index_buffer() const;

private:
    std::vector<vertex3d> _vertices;
    std::vector<int> _indices;
    std::vector<submesh_info> _submeshes;

    vao_map _vao;
    graphics_buffer _vbo { graphics_buffer::type::vertex };
    graphics_buffer _ebo { graphics_buffer::type::index };
};
} // namespace graphics
