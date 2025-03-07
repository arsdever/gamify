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

std::any default_property_value_for_type(graphics::shader_property_type type)
{
    switch (type)
    {
    case graphics::shader_property_type::spt_float: return 0.0f;
    case graphics::shader_property_type::spt_vec2: return glm::vec2(0.0f);
    case graphics::shader_property_type::spt_vec3: return glm::vec3(0.0f);
    case graphics::shader_property_type::spt_vec4: return glm::vec4(0.0f);
    case graphics::shader_property_type::spt_double: return 0.0;
    case graphics::shader_property_type::spt_dvec2: return glm::dvec2(0.0);
    case graphics::shader_property_type::spt_dvec3: return glm::dvec3(0.0);
    case graphics::shader_property_type::spt_dvec4: return glm::dvec4(0.0);
    case graphics::shader_property_type::spt_int: return 0;
    case graphics::shader_property_type::spt_ivec2: return glm::ivec2(0);
    case graphics::shader_property_type::spt_ivec3: return glm::ivec3(0);
    case graphics::shader_property_type::spt_ivec4: return glm::ivec4(0);
    case graphics::shader_property_type::spt_unsigned_int: return 0u;
    case graphics::shader_property_type::spt_uvec2: return glm::uvec2(0u);
    case graphics::shader_property_type::spt_uvec3: return glm::uvec3(0u);
    case graphics::shader_property_type::spt_uvec4: return glm::uvec4(0u);
    case graphics::shader_property_type::spt_bool: return false;
    case graphics::shader_property_type::spt_bvec2: return glm::bvec2(false);
    case graphics::shader_property_type::spt_bvec3: return glm::bvec3(false);
    case graphics::shader_property_type::spt_bvec4: return glm::bvec4(false);
    case graphics::shader_property_type::spt_mat2: return glm::mat2(1.0f);
    case graphics::shader_property_type::spt_mat3: return glm::mat3(1.0f);
    case graphics::shader_property_type::spt_mat4: return glm::mat4(1.0f);
    case graphics::shader_property_type::spt_mat2x3: return glm::mat2x3(1.0f);
    case graphics::shader_property_type::spt_mat2x4: return glm::mat2x4(1.0f);
    case graphics::shader_property_type::spt_mat3x2: return glm::mat3x2(1.0f);
    case graphics::shader_property_type::spt_mat3x4: return glm::mat3x4(1.0f);
    case graphics::shader_property_type::spt_mat4x2: return glm::mat4x2(1.0f);
    case graphics::shader_property_type::spt_mat4x3: return glm::mat4x3(1.0f);
    case graphics::shader_property_type::spt_sampler2D:
        return std::shared_ptr<texture>();
    default: return {};
    }
}

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
    if (auto prog = _shader.lock())
    {
        if (prog->is_valid())
            return prog;
    }

    if (_fallback_shader == nullptr)
    {
        _fallback_shader =
            assets::asset_manager::get<shader>("standard.fallback.shader");
    }

    return _fallback_shader;
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
    program()->visit_properties(
        [ visitor = std::move(visitor), this ](shader_property& p)
    {
        const auto& value = _property_map[ p.name ];
        if (value.has_value())
        {
            visitor(p.name, value);
            return;
        }

        auto type = p.type;
        visitor(p.name, default_property_value_for_type(type));
    });
}

std::shared_ptr<graphics::shader> material::_fallback_shader = {};
} // namespace graphics
