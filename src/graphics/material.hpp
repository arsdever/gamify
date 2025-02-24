#pragma once

#include "graphics/graphics_fwd.hpp"

#include "common/utils.hpp"

namespace graphics
{
class material : public std::enable_shared_from_this<material>
{
private:
    using property_map_t =
        std::unordered_map<std::string, std::any, string_hash, std::equal_to<>>;

public:
    material();
    material(material&& mat);
    material(const material& mat) = delete;
    material& operator=(material&& mat);
    material& operator=(const material& mat) = delete;
    ~material();

    std::shared_ptr<graphics::shader> program() const;
    void set_shader_program(std::shared_ptr<graphics::shader> prog);

    void set_property_value(std::string_view name, std::any value);

    std::optional<std::any> get_property_value(std::string_view name) const;

    template <typename T>
    std::optional<T> get_property_value(std::string_view name)
    {
        auto value = get_property_value(name);
        if (value.has_value() && value->type() == typeid(T))
            return std::any_cast<T>(value.value());

        return std::nullopt;
    }

    template <typename... T>
    void set_property_value(std::string_view name, T... args)
    {
        set_property_value(name, std::any { std::make_tuple(args...) });
    }

    template <typename T>
    void set_property_value(std::string_view name, T element)
    {
        set_property_value(name, std::any { element });
    }

    void activate() const;
    void deactivate() const;

    std::shared_ptr<material> clone() const;

    static std::shared_ptr<material>
    from_shader(std::shared_ptr<graphics::shader> shader);
    static void set_fallback_shader(std::shared_ptr<graphics::shader> shader);

private:
    std::weak_ptr<graphics::shader> _shader;
    property_map_t _property_map;
    unsigned _textures_count { 0 };
    static std::shared_ptr<graphics::shader> _fallback_shader;
};
} // namespace graphics
