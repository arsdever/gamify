#include <glad/gl.h>

#include "graphics/shader.hpp"

#include "common/logging.hpp"
#include "graphics/shader_script.hpp"

namespace graphics
{
namespace
{
static inline logger log() { return get_logger("shader"); }
} // namespace

shader::shader() { _id = glCreateProgram(); }

shader::shader(shader&& other)
{
    _id = other._id;
    _shaders = std::move(other._shaders);
    _properties = std::move(other._properties);
    _name_property_map = std::move(other._name_property_map);
    other._id = -1;
}

shader& shader::operator=(shader&& other)
{
    _id = other._id;
    _shaders = std::move(other._shaders);
    _properties = std::move(other._properties);
    _name_property_map = std::move(other._name_property_map);
    other._id = -1;
    return *this;
}

shader::~shader() = default;

void shader::compile()
{
    for (auto& ss : _shaders)
    {
        ss->compile();
        glAttachShader(_id, ss->id());
    }

    glLinkProgram(_id);

    int error_code;
    glGetProgramiv(_id, GL_LINK_STATUS, &error_code);

    if (error_code == GL_FALSE)
    {
        int log_length = 0;
        char msg[ 1024 ];
        glGetProgramInfoLog(_id, 1024, &log_length, msg);
        log()->error("Failed to compile shader {}({}): {}", _id, _name, msg);
        _is_valid = false;

        for (auto& ss : _shaders)
        {
            glDetachShader(_id, ss->id());
        }
        return;
    }

    for (auto& ss : _shaders)
    {
        glDetachShader(_id, ss->id());
    }

    _is_valid = true;
    resolve_uniforms();
}

void shader::activate()
{
    glUseProgram(id());
    setup_property_values();
}

void shader::add_shader(std::shared_ptr<shader_script> shader)
{
    _shaders.push_back(shader);
}

void shader::add_shader(std::string_view shader_script_path)
{
    auto ss = std::make_shared<shader_script>(
        std::move(shader_script::from_file(shader_script_path)));
    add_shader(ss);
}

void shader::release_shaders() { _shaders.clear(); }

int shader::id() const { return _id; }

void shader::set_name(std::string name) { _name = std::move(name); }

std::string shader::get_name() const { return _name; }

void shader::set_property(const char* name, std::any value)
{
    set_property(std::string(name), std::move(value));
}

void shader::set_property(std::string_view name, std::any value)
{
    set_property(std::string(name), std::move(value));
}

void shader::set_property(std::string name, std::any value)
{
    if (_name_property_map.find(name) == _name_property_map.end())
        return;

    _name_property_map.at(name).value = std::move(value);
}

void shader::visit_properties(std::function<void(shader_property&)> visitor)
{
    for (auto& prop : _properties)
    {
        visitor(prop);
    }
}

void shader::visit_properties(
    std::function<void(const shader_property&)> visitor) const
{
    for (const auto& prop : _properties)
    {
        visitor(prop);
    }
}

bool shader::is_valid() const { return _is_valid; }

void shader::resolve_uniforms()
{
    glUseProgram(id());

    _properties.clear();
    _name_property_map.clear();

    int uniform_count = 0;
    glGetProgramiv(id(), GL_ACTIVE_UNIFORMS, &uniform_count);
    if (uniform_count == 0)
        return;

    std::string buffer;
    int length;
    int size;
    unsigned type;
    buffer.resize(512);
    _properties.resize(uniform_count);
    for (int i = 0; i < uniform_count; ++i)
    {
        glGetActiveUniform(id(), i, 512, &length, &size, &type, buffer.data());
        _properties[ i ].name = buffer;
        _properties[ i ].name.resize(length);
        _properties[ i ].location_info = i;
        _properties[ i ].size = size;
        // TODO: verify emplace did add element
        _name_property_map.try_emplace(_properties[ i ].name, _properties[ i ]);
    }
}

void shader::setup_property_values() const
{
    for (const auto& prop : _properties)
    {
        const auto& v = prop.value;
        if (!v.has_value())
        {
            continue;
        }

        auto location = prop.location_info;
        if (v.type() == typeid(float))
        {
            glUniform1f(location, std::any_cast<float>(v));
        }
        else if (v.type() == typeid(std::tuple<float>))
        {
            auto [ v0 ] = std::any_cast<std::tuple<float>>(v);
            glUniform1f(location, v0);
        }
        else if (v.type() == typeid(std::tuple<float, float>))
        {
            auto [ v0, v1 ] = std::any_cast<std::tuple<float, float>>(v);
            glUniform2f(location, v0, v1);
        }
        else if (v.type() == typeid(std::tuple<float, float, float>))
        {
            auto [ v0, v1, v2 ] =
                std::any_cast<std::tuple<float, float, float>>(v);
            glUniform3f(location, v0, v1, v2);
        }
        else if (v.type() == typeid(std::tuple<float, float, float, float>))
        {
            auto [ v0, v1, v2, v3 ] =
                std::any_cast<std::tuple<float, float, float, float>>(v);
            glUniform4f(location, v0, v1, v2, v3);
        }
        else if (v.type() == typeid(int))
        {
            glUniform1i(location, std::any_cast<int>(v));
        }
        else if (v.type() == typeid(std::tuple<int>))
        {
            auto [ v0 ] = std::any_cast<std::tuple<int>>(v);
            glUniform1i(location, v0);
        }
        else if (v.type() == typeid(std::tuple<int, int>))
        {
            auto [ v0, v1 ] = std::any_cast<std::tuple<int, int>>(v);
            glUniform2i(location, v0, v1);
        }
        else if (v.type() == typeid(std::tuple<int, int, int>))
        {
            auto [ v0, v1, v2 ] = std::any_cast<std::tuple<int, int, int>>(v);
            glUniform3i(location, v0, v1, v2);
        }
        else if (v.type() == typeid(std::tuple<int, int, int, int>))
        {
            auto [ v0, v1, v2, v3 ] =
                std::any_cast<std::tuple<int, int, int, int>>(v);
            glUniform4i(location, v0, v1, v2, v3);
        }
        else if (v.type() == typeid(unsigned))
        {
            glUniform1ui(location, std::any_cast<unsigned>(v));
        }
        else if (v.type() == typeid(std::tuple<unsigned>))
        {
            auto [ v0 ] = std::any_cast<std::tuple<unsigned>>(v);
            glUniform1ui(location, v0);
        }
        else if (v.type() == typeid(std::tuple<unsigned, unsigned>))
        {
            auto [ v0, v1 ] = std::any_cast<std::tuple<unsigned, unsigned>>(v);
            glUniform2ui(location, v0, v1);
        }
        else if (v.type() == typeid(std::tuple<unsigned, unsigned, unsigned>))
        {
            auto [ v0, v1, v2 ] =
                std::any_cast<std::tuple<unsigned, unsigned, unsigned>>(v);
            glUniform3ui(location, v0, v1, v2);
        }
        else if (v.type() ==
                 typeid(std::tuple<unsigned, unsigned, unsigned, unsigned>))
        {
            auto [ v0, v1, v2, v3 ] = std::any_cast<
                std::tuple<unsigned, unsigned, unsigned, unsigned>>(v);
            glUniform4ui(location, v0, v1, v2, v3);
        }
        else if (v.type() == typeid(glm::vec1))
        {
            glUniform1f(location, std::any_cast<glm::vec1>(v).x);
        }
        else if (v.type() == typeid(glm::vec2))
        {
            glUniform2f(location,
                        std::any_cast<glm::vec2>(v).x,
                        std::any_cast<glm::vec2>(v).y);
        }
        else if (v.type() == typeid(glm::vec3))
        {
            glUniform3f(location,
                        std::any_cast<glm::vec3>(v).x,
                        std::any_cast<glm::vec3>(v).y,
                        std::any_cast<glm::vec3>(v).z);
        }
        else if (v.type() == typeid(glm::vec4))
        {
            glUniform4f(location,
                        std::any_cast<glm::vec4>(v).x,
                        std::any_cast<glm::vec4>(v).y,
                        std::any_cast<glm::vec4>(v).z,
                        std::any_cast<glm::vec4>(v).w);
        }
        else if (v.type() == typeid(glm::uvec1))
        {
            glUniform1ui(location, std::any_cast<glm::uvec1>(v).x);
        }
        else if (v.type() == typeid(glm::uvec2))
        {
            glUniform2ui(location,
                         std::any_cast<glm::uvec2>(v).x,
                         std::any_cast<glm::uvec2>(v).y);
        }
        else if (v.type() == typeid(glm::uvec3))
        {
            glUniform3ui(location,
                         std::any_cast<glm::uvec3>(v).x,
                         std::any_cast<glm::uvec3>(v).y,
                         std::any_cast<glm::uvec3>(v).z);
        }
        else if (v.type() == typeid(glm::uvec4))
        {
            glUniform4ui(location,
                         std::any_cast<glm::uvec4>(v).x,
                         std::any_cast<glm::uvec4>(v).y,
                         std::any_cast<glm::uvec4>(v).z,
                         std::any_cast<glm::uvec4>(v).w);
        }
        else if (v.type() == typeid(glm::ivec1))
        {
            glUniform1i(location, std::any_cast<glm::ivec1>(v).x);
        }
        else if (v.type() == typeid(glm::ivec2))
        {
            glUniform2i(location,
                        std::any_cast<glm::ivec2>(v).x,
                        std::any_cast<glm::ivec2>(v).y);
        }
        else if (v.type() == typeid(glm::ivec3))
        {
            glUniform3i(location,
                        std::any_cast<glm::ivec3>(v).x,
                        std::any_cast<glm::ivec3>(v).y,
                        std::any_cast<glm::ivec3>(v).z);
        }
        else if (v.type() == typeid(glm::ivec4))
        {
            glUniform4i(location,
                        std::any_cast<glm::ivec4>(v).x,
                        std::any_cast<glm::ivec4>(v).y,
                        std::any_cast<glm::ivec4>(v).z,
                        std::any_cast<glm::ivec4>(v).w);
        }
        else if (v.type() == typeid(std::tuple<glm::mat2>))
        {
            auto [ value ] = std::any_cast<std::tuple<glm::mat2>>(v);
            glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(value));
        }
        else if (v.type() == typeid(std::tuple<glm::mat3>))
        {
            auto [ value ] = std::any_cast<std::tuple<glm::mat3>>(v);
            glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
        }
        else if (v.type() == typeid(std::tuple<glm::mat4>))
        {
            auto [ value ] = std::any_cast<std::tuple<glm::mat4>>(v);
            glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
        }
        else if (v.type() == typeid(glm::mat2))
        {
            glUniformMatrix2fv(location,
                               1,
                               GL_FALSE,
                               glm::value_ptr(std::any_cast<glm::mat2>(v)));
        }
        else if (v.type() == typeid(glm::mat3))
        {
            glUniformMatrix3fv(location,
                               1,
                               GL_FALSE,
                               glm::value_ptr(std::any_cast<glm::mat3>(v)));
        }
        else if (v.type() == typeid(glm::mat4))
        {
            glUniformMatrix4fv(location,
                               1,
                               GL_FALSE,
                               glm::value_ptr(std::any_cast<glm::mat4>(v)));
        }
        else
        {
            log()->warn("Unknown uniform type '{}' specified for property {}",
                        v.type().name(),
                        prop.name);
        }
    }
}
} // namespace graphics
