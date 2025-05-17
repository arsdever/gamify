#include <nlohmann/json.hpp>

#include "project/components/light.hpp"

#include "common/logging.hpp"
#include "project/serializer_json.hpp"

namespace
{
static logger log() { return get_logger("light"); }
} // namespace

namespace components
{
light::light(game_object& obj)
    : component(type_name, obj)
{
    add_property(
        { "color", "Color", "Light color", glm::dvec4(1.0, 1.0, 1.0, 1.0) });
    add_property({ "intensity", "Intensity", "Light intensity", 1.0 });
    add_property({ "radius", "Radius", "Light radius", 1.0 });
    add_property({ "light_type",
                   "Light Type",
                   "Light type",
                   static_cast<int>(type::OMNI) });
}

light::~light() = default;

glm::dvec4 light::get_color() const
{
    return get_property("color").get_value<glm::dvec4>();
}

void light::set_color(glm::dvec4 color) { get_property("color") = color; }

float light::get_intensity() const
{
    return get_property("intensity").get_value<double>();
}

void light::set_intensity(float intensity)
{
    get_property("intensity") = static_cast<double>(intensity);
}

double light::get_radius() const
{
    return get_property("radius").get_value<double>();
}

void light::set_radius(double radius) { get_property("radius") = radius; }

light::type light::get_type() const
{
    return static_cast<light::type>(
        get_property("light_type").get_value<int>());
}

void light::set_type(type light_type)
{
    get_property("light_type") = light_type;
}

template <>
void light::serialize(json_serializer& j)
{
    j.add_component(nlohmann::json {
        { "type", light::type_name },
        { "is_enabled", is_enabled() },
    });
}

void light::deserialize(const nlohmann::json& j)
{
    set_enabled(j[ "is_enabled" ]);
}
} // namespace components
