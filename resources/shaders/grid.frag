#version 460 core

in vec2 fragment_position;
flat in int line_segment_index;

uniform float u_grid_size;
uniform int u_grid_count;

out vec4 o_color;

vec3 lighten(vec3 color, float percent) { return color * percent; }

float is_nth(int i, int n) { return (1 - sign(i % n - .5)) / 2.0; }

void main()
{
    vec3 color = vec3(0.4);
    const float max_distance = (u_grid_count / 4) * u_grid_size;

    float nth = is_nth(abs(line_segment_index), 10);
    color = lighten(color, 1.0 + nth * .5);

    float falloff = 1 - distance(fragment_position, vec2(0)) / max_distance;
    falloff = clamp(falloff, 0, 1);

    o_color = vec4(color, falloff);
}
