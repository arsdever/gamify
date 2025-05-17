#version 460 core

// expected coordinates from (-1,-1) to (1,1)
layout(location = 0) in vec3 i_vertex_position;
// expected coordinates from (0,0) to (1,1)
layout(location = 2) in vec2 i_vertex_uv;
layout(location = 3) in vec4 i_vertex_color;

uniform mat4 u_vp_matrix = mat4(1);
uniform mat4 u_model_matrix = mat4(1);

out vec2 fragment_uv;
out vec4 fragment_color;

void main()
{
    mat4 mvp = u_vp_matrix * u_model_matrix;
    gl_Position = mvp * vec4(i_vertex_position, 1.0);
    fragment_uv = i_vertex_uv;
    fragment_color = i_vertex_color;
}
