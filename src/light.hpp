#pragma once

#include "transform.hpp"

class image;
class texture;
class mesh;
class shader_program;

class light
{
public:
    enum class type
    {
        OMNI,
        SPOTLIGHT,
        SPHERICAL,
        DIRECTIONAL
    };

public:
    light();
    ~light();

    glm::vec3 get_color() const;
    void set_color(glm::vec3 color);

    float get_intensity() const;
    void set_intensity(float intensity);

    float get_radius() const;
    void set_radius(float radius);

    type get_type() const;
    void set_type(type light_type);

    transform& get_transform();
    const transform& get_transform() const;

    static const std::vector<light*>& get_all_lights();

private:
    transform _transformation;
    glm::vec3 _color { 1.0f, 1.0f, 1.0f };
    float _intensity { 1.0f };
    type _light_type { type::OMNI };
    float _radius = 1.0f;
    static std::vector<light*> _lights;
};
