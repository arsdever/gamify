#include <glad/gl.h>

#include "graphics/mesh.hpp"

namespace graphics
{
void mesh::init()
{
    _vbo.set_element_stride(vertex3d::size);
    _vbo.set_element_count(_vertices.size());
    _vbo.set_data(_vertices.data());

    _ebo.set_element_stride(sizeof(int));
    _ebo.set_element_count(_indices.size());
    _ebo.set_data(_indices.data());
}

void mesh::set_vertices(std::vector<vertex3d> positions)
{
    _vertices = std::move(positions);
}

void mesh::set_indices(std::vector<int> indices)
{
    _indices = std::move(indices);
}

void mesh::recalculate_normals()
{
    for (auto v : _vertices)
    {
        v.normal() = glm::vec3(0.0f);
    }

    for (size_t i = 0; i < _indices.size(); i += 3)
    {
        glm::vec3 face_normal =
            glm::cross(_vertices[ _indices[ i + 1 ] ].position() -
                           _vertices[ _indices[ i ] ].position(),
                       _vertices[ _indices[ i + 2 ] ].position() -
                           _vertices[ _indices[ i ] ].position());

        _vertices[ _indices[ i ] ].normal() += face_normal;
        _vertices[ _indices[ i + 1 ] ].normal() += face_normal;
        _vertices[ _indices[ i + 2 ] ].normal() += face_normal;
    }

    for (auto& v : _vertices)
    {
        v.normal() = glm::normalize(v.normal());
    }
}

void mesh::set_submeshes(std::vector<submesh_info> submeshes)
{
    _submeshes = std::move(submeshes);
}

void mesh::apply_submeshes(std::function<void(submesh_info&)> func)
{
    for (auto& submesh : _submeshes)
    {
        func(submesh);
    }
}

void mesh::render()
{
    if (_vao.activate())
    {
        glBindBuffer(GL_ARRAY_BUFFER, _vbo.get_handle());
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo.get_handle());
        vertex3d::initialize_attributes();
    }

    vertex3d::activate_attributes();
    glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

const graphics_buffer& mesh::get_vertex_buffer() const { return _vbo; }

const graphics_buffer& mesh::get_index_buffer() const { return _ebo; }
} // namespace graphics
