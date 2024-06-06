#version 460 core

layout(location = 0) in vec3 i_vertex_position;

uniform mat4 u_model_matrix;
uniform mat4 u_vp_matrix;

void main()
{
    gl_Position = u_vp_matrix * u_model_matrix * vec4(i_vertex_position, 1.0);
}
