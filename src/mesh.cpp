#include "mesh.hpp"

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

void mesh::set_submeshes(std::vector<submesh_info> submeshes)
{
    _submeshes = std::move(submeshes);
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
