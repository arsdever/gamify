#version 460 core

layout(location = 0) in vec2 i_vertex_position;

uniform vec3 u_grid_origin;
uniform mat4 u_model_matrix;
uniform mat4 u_vp_matrix;
uniform float u_grid_size;
uniform int u_grid_count;

out vec2 fragment_position;
flat out int line_segment_index;

void main()
{
    const int grid_count_half = u_grid_count / 2;
    const int grid_count_quarter = u_grid_count / 4;

    /*
       Determine, whether the x and y coordinates should be swapped.
       Basically, the upper half number of instances will be used as y axis
       grid.
    */
    int switch_factor = gl_InstanceID / grid_count_half;

    /*
       Calculates the the index of the line in the axis range. Counts from 0.
    */
    int index_per_axis = gl_InstanceID % grid_count_half;

    /*
       Calculate the displacement across the axis.
    */
    int symmetric_index = index_per_axis - grid_count_quarter;
    float position_per_axis = symmetric_index * u_grid_size;

    vec2 pos = i_vertex_position;
    pos.x = position_per_axis;

    /*
       Calculate the length of each line, so that the grid closes complete
    */
    pos.y *= u_grid_size * grid_count_quarter;

    /*
       Based on the instance id, determine whether this is an x axis or y axis
       grid line
    */
    pos = mix(pos.xy, pos.yx, switch_factor);

    mat4 mvp = u_vp_matrix * u_model_matrix;
    gl_Position = mvp * vec4(pos.x, 0, pos.y, 1);
    fragment_position = pos;
    line_segment_index = symmetric_index;
}
