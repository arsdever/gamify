#include "graphics/material.hpp"

#include "assets/asset_manager.hpp"
#include "common/logging.hpp"
#include "graphics/shader.hpp"
#include "graphics/texture.hpp"

namespace graphics
{
namespace
{
static inline logger log() { return get_logger("material"); }
} // namespace

material::material() = default;

material::material(material&& mat)
{
    _shader = mat._shader;
    _property_map = std::move(mat._property_map);
    mat._shader = {};
}

material& material::operator=(material&& mat)
{
    _shader = mat._shader;
    _property_map = std::move(mat._property_map);
    mat._shader = {};
    return *this;
}

material::~material() = default;

std::shared_ptr<graphics::shader> material::program() const
{
    return _shader.lock();
}

void material::set_shader_program(std::shared_ptr<graphics::shader> prog)
{
    _shader = prog;
}

std::optional<std::any>
material::get_property_value(std::string_view name) const
{
    auto it = _property_map.find(std::string(name));
    if (it == _property_map.end())
        return std::nullopt;

    return it->second;
}

void material::set_property_value(std::string_view name, std::any value)
{
    _property_map[ std::string(name) ] = std::move(value);
}

void material::activate() const
{
    auto s = program();
    if (!s)
    {
        if (_fallback_shader)
        {
            _fallback_shader->activate();
        }
        return;
    }

    int texture_binding_index = 0;
    for (const auto& [ name, value ] : _property_map)
    {
        if (value.type() == typeid(std::shared_ptr<graphics::texture>))
        {
            auto t = std::any_cast<std::shared_ptr<graphics::texture>>(value);
            t->set_active_texture(texture_binding_index);
            s->set_property(name, texture_binding_index++);
        }
        else if (value.type() == typeid(texture*))
        {
            const auto* t = std::any_cast<texture*>(value);
            t->set_active_texture(texture_binding_index);
            s->set_property(name, texture_binding_index++);
        }
        else
        {
            s->set_property(name, value);
        }
    }

    s->activate();
}

void material::deactivate() const { }

std::shared_ptr<material> material::clone() const
{
    auto result = std::make_shared<material>();
    result->_shader = _shader;
    result->_property_map = _property_map;
    result->_textures_count = _textures_count;

    return result;
}

std::shared_ptr<material>
material::from_shader(std::shared_ptr<graphics::shader> shader)
{
    auto mat = std::make_shared<material>();
    mat->set_shader_program(shader);
    return mat;
}

void material::set_fallback_shader(std::shared_ptr<graphics::shader> shader)
{
    _fallback_shader = std::move(shader);
}

void material::visit_properties(
    std::function<void(std::string_view property_name,
                       const std::any& property_value)> visitor)
{
    for (const auto& [ name, value ] : _property_map)
    {
        visitor(name, value);
    }
}

std::shared_ptr<graphics::shader> material::_fallback_shader = {};
} // namespace graphics
