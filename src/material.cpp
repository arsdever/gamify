#include <glad/gl.h>
#include <glm/ext.hpp>

#include "material.hpp"

#include "logging.hpp"
#include "shader.hpp"

namespace
{
static inline logger log() { return get_logger("material"); }
} // namespace

material::material() = default;

material::material(material&& mat)
{
    _shader_program = mat._shader_program;
    mat._shader_program = 0;
}

material& material::operator=(material&& mat)
{
    _shader_program = mat._shader_program;
    mat._shader_program = 0;
    return *this;
}

material::~material() = default;

shader_program* material::program() const { return _shader_program; }

void material::set_shader_program(shader_program* prog)
{
    _shader_program = prog;
}

void material::declare_property(std::string_view name,
                                material_property::data_type type)
{
    material_property property;
    property._name = name;
    property._type = type;
    if (auto [ existing, success ] =
            _property_map.try_emplace(std::string(name), std::move(property));
        !success)
    {
        std::string_view reason = "Unknown";
        if (existing != _property_map.end())
        {
            reason = "Another property with the same name already exists";
        }
        log()->error("Failed to add property \"{}\": {}", name, reason);
    }
}

bool material::has_property(std::string_view name) const
{
    return _property_map.contains(name);
}

void material::set_property_value(std::string_view name, std::any value)
{
    if (!has_property(name))
    {
        log()->error("The material has no property \"{}\"", name);
        return;
    }

    // not using unordered_map.at to have generic string comparison
    property_map_t::iterator found_iterator = _property_map.find(name);
    found_iterator->second._value = std::move(value);
}

void material::activate() { }
