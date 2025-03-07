#version 460 core

in vec3 fragment_position;
in vec3 fragment_normal;
in vec2 fragment_uv;
in vec3 fragment_tangent;
in vec3 fragment_bitangent;

layout(location = 1, binding = 0) uniform sampler2D u_albedo_texture;
layout(location = 2) uniform float u_albedo_texture_strength;
layout(location = 3) uniform vec4 u_albedo_color;
layout(location = 4, binding = 1) uniform sampler2D u_normal_texture;
layout(location = 5) uniform float u_normal_texture_strength;
layout(location = 6) uniform float u_metallic;
layout(location = 7, binding = 2) uniform sampler2D u_metallic_texture;
layout(location = 8) uniform float u_metallic_texture_strength;
layout(location = 9, binding = 3) uniform sampler2D u_roughness_texture;
layout(location = 10) uniform float u_roughness_texture_strength;
layout(location = 11) uniform float u_roughness;
layout(location = 12) uniform float u_ambient_occlusion;

uniform vec3 u_camera_position;

#define LIGHT_OMNI  0
#define SPOTLIGHT   1
#define SPHERICAL   2
#define DIRECTIONAL 3
const float PI = 3.14159265359;

struct light_t
{
    vec3 position;
    float intensity;
    vec3 direction;
    float radius;
    vec3 color;
    uint type;
};

layout(std430, binding = 0) buffer lights_buffer { light_t[] lights; };

out vec4 fragment_color;

vec2 confragment_from_blenders_uv_map(vec2 blender_uv)
{
    // blender's uv map coordinates range is from bottom-left
    // we need to convert them to top-left
    // (0,1) - (1,1)        (0,0) - (1,0)
    //   -       -     =>     -       -
    // (0,0) - (1,0)        (0,1) - (1,1)
    return vec2(fragment_uv.x, 1.0 - fragment_uv.y);
}

vec4 albedo_mixed_color(vec2 uv_coord)
{
    return mix(u_albedo_color,
               texture(u_albedo_texture, uv_coord),
               u_albedo_texture_strength);
}

vec3 surface_normal(vec2 uv_coord) { return fragment_normal; }

vec3 calculate_light_ambient(light_t light)
{
    vec3 ambient = 0.1 * light.color;
    return ambient;
}

vec3 fresnel_schlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float ggx_distribution(vec3 vertex_normal, vec3 half_vector, float roughness)
{
    float roughness_2 = roughness * roughness;
    float numerator = roughness_2;

    float roughness_4 = roughness_2 * roughness_2;
    float reflection_angle_match_coefficient =
        max(dot(vertex_normal, half_vector), 0.0);
    float reflection_angle_match_coefficient_square =
        reflection_angle_match_coefficient * reflection_angle_match_coefficient;
    float denominator =
        (reflection_angle_match_coefficient * (roughness_2 - 1.0) + 1.0);
    denominator = PI * denominator * denominator;

    return numerator / denominator;
}

float ggx_geometry_schlick(float vector_match, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float numerator = vector_match;
    float denominator = vector_match * (1.0 - k) + k;

    return numerator / denominator;
}

float smiths_geometry_function(vec3 fragment_normal,
                               vec3 view_direction,
                               vec3 light_direction,
                               float roughness)
{
    float view_to_surface_direction_match =
        max(dot(fragment_normal, view_direction), 0.0);
    float light_to_surface_direction_match =
        max(dot(fragment_normal, light_direction), 0.0);
    float ggx2 =
        ggx_geometry_schlick(view_to_surface_direction_match, roughness);
    float ggx1 =
        ggx_geometry_schlick(light_to_surface_direction_match, roughness);

    return ggx1 * ggx2;
}

vec3 direct_light_contribution(light_t light,
                               vec3 surface_albedo,
                               vec3 surface_normal,
                               vec3 view_direction,
                               vec3 direct_fresnel,
                               float roughness,
                               float metallic)
{
    // calculate per-light radiance
    vec3 light_to_fragment_direction =
        normalize(light.position - fragment_position);
    vec3 half_vector = normalize(view_direction + light_to_fragment_direction);
    float light_to_fragment_distance =
        length(light.position - fragment_position);
    float attenuation = light.intensity / (light_to_fragment_distance *
                                           light_to_fragment_distance);
    vec3 radiance = light.color * attenuation;

    // cook-torrance brdf
    float normal_distribution =
        ggx_distribution(surface_normal, half_vector, roughness);
    float geometry_function = smiths_geometry_function(
        surface_normal, view_direction, light_to_fragment_direction, roughness);
    vec3 fresnel = fresnel_schlick(max(dot(half_vector, view_direction), 0.0),
                                   direct_fresnel);

    vec3 specular_light_coefficient = fresnel;
    vec3 direct_light_coefficient = vec3(1.0) - specular_light_coefficient;
    direct_light_coefficient *= 1.0 - metallic;

    vec3 numerator = normal_distribution * geometry_function * fresnel;
    float denominator =
        4.0 * max(dot(surface_normal, view_direction), 0.0) *
            max(dot(surface_normal, light_to_fragment_direction), 0.0) +
        0.0001;
    vec3 specular = numerator / denominator;

    // add to outgoing radiance Lo
    float NdotL = max(dot(surface_normal, light_to_fragment_direction), 0.0);

    return (direct_light_coefficient * surface_albedo / PI + specular) *
           radiance * NdotL;
}

void main()
{
    vec3 surface_normal = surface_normal(fragment_uv);
    vec3 view_direction = normalize(u_camera_position - fragment_position);

    vec3 direct_fresnel = vec3(0.04);
    vec3 albedo = albedo_mixed_color(fragment_uv).xyz;
    float roughness = mix(u_roughness,
                          texture(u_roughness_texture, fragment_uv).r,
                          u_roughness_texture_strength);
    float metallic = mix(u_metallic,
                         texture(u_metallic_texture, fragment_uv).r,
                         u_metallic_texture_strength);
    direct_fresnel = mix(direct_fresnel, albedo.rgb, metallic);

    // reflectance equation
    vec3 direct_light = vec3(0.0);
    for (int i = 0; i < lights.length(); ++i)
    {
        direct_light += direct_light_contribution(lights[ i ],
                                                  albedo,
                                                  surface_normal,
                                                  view_direction,
                                                  direct_fresnel,
                                                  roughness,
                                                  metallic);
    }

    vec3 ambient = vec3(0.03) * albedo * u_ambient_occlusion;
    vec3 color = ambient + direct_light;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    fragment_color = vec4(color, 1.0);
}
