#pragma once

#include "graphics/graphics_fwd.hpp"

#include "common/utils.hpp"
#include "graphics/shader_property.hpp"

namespace graphics
{
class shader : public std::enable_shared_from_this<shader>
{
public:
    shader();
    shader(const shader& other) = delete;
    shader(shader&& other);
    shader& operator=(const shader& other) = delete;
    shader& operator=(shader&& other);
    ~shader();

    void compile();
    void activate();

    void add_shader(std::shared_ptr<shader_script> shader);
    void add_shader(std::string_view shader_script_path);
    void release_shaders();

    int id() const;

    void set_name(std::string name);
    std::string get_name() const;

    void set_property(const char* name, std::any value);
    void set_property(std::string_view name, std::any value);
    void set_property(std::string name, std::any value);
    void visit_properties(std::function<void(shader_property&)> visitor);
    void
    visit_properties(std::function<void(const shader_property&)> visitor) const;

    bool has_property(std::string_view name) const;

    shader_property& get_property(std::string_view name);
    const shader_property& get_property(std::string_view name) const;

    bool is_valid() const;

private:
    void resolve_uniforms();
    void setup_property_values() const;

private:
    int _id { 0 };
    std::vector<std::shared_ptr<shader_script>> _shaders;
    std::string _name;
    bool _is_valid { false };

    std::vector<shader_property> _properties;
    std::unordered_map<std::string,
                       shader_property&,
                       string_hash,
                       std::equal_to<>>
        _name_property_map;
};
} // namespace graphics
