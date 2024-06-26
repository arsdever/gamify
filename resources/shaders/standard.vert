#version 460 core

layout(location = 0) in vec3 i_vertex_position;
layout(location = 1) in vec3 i_vertex_normal;
layout(location = 2) in vec2 i_vertex_uv;
layout(location = 3) in vec3 i_vertex_tangent;
layout(location = 4) in vec3 i_vertex_bitangent;

uniform mat4 u_model_matrix;
uniform mat4 u_vp_matrix;

out vec3 fragment_position;
out vec3 fragment_normal;
out vec2 fragment_uv;
out vec3 fragment_tangent;
out vec3 fragment_bitangent;

void main()
{
    mat4 mvp = u_vp_matrix * u_model_matrix;
    gl_Position = mvp * vec4(i_vertex_position, 1.0);
    fragment_position = vec3(u_model_matrix * vec4(i_vertex_position, 1.0));
    fragment_normal =
        mat3(transpose(inverse(u_model_matrix))) * i_vertex_normal;
    fragment_uv = i_vertex_uv;
    fragment_tangent = i_vertex_tangent;
    fragment_bitangent = i_vertex_bitangent;
}
