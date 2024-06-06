#version 460 core

uniform vec4 u_color = vec4(1, 0, 1, 1);

out vec4 o_color;

void main() { o_color = u_color; }
