#pragma once

#include "project/project_fwd.hpp"

#include "project/component_interface/metatype.hpp"
#include "project/definitions.hpp"
#include "project/game_object.hpp"
#include "project/object.hpp"
#include "project/property.hpp"

class component : public object
{
public:
    using property_const_visitor_type = std::function<bool(const property&)>;
    using property_visitor_type = std::function<bool(property&)>;

public:
    game_object& get_game_object() const;

    std::shared_ptr<components::transform> get_transform() const;

    std::shared_ptr<component> get(std::string_view type_name);

    template <typename T>
    std::shared_ptr<T> get()
    {
        if constexpr (std::is_same_v<T, components::transform>)
        {
            return get_transform();
        }

        return static_cast<std::shared_ptr<T>>(get(T::type_name));
    }

    inline void enable() { set_enabled(true); }
    inline void disable() { set_enabled(false); }
    void set_enabled(bool active = true);
    bool is_enabled() const;

    bool set_property_value(std::string_view name, std::any value);
    template <typename T>
    T get_property_value(std::string_view name) const
    {
        auto& prop = get_property(name);
        return prop.get_value<T>();
    }

    void for_each_property(const property_const_visitor_type& visitor) const;
    void for_each_property(const property_visitor_type& visitor);
    std::string name() const;

    void init();
    void update();
    void deinit();

    bool can_cast(const metatype& to_type) const;
    bool can_cast(std::string_view type_name) const;

protected:
    component(std::string_view name, game_object& obj);

    property& add_property(property&& prop);
    const property& get_property(std::string_view name) const;
    const property* try_get_property(std::string_view name) const;
    property& get_property(std::string_view name);
    property* try_get_property(std::string_view name);

    virtual void on_init();
    virtual void on_update();
    virtual void on_deinit();

    std::reference_wrapper<game_object> _game_object;

private:
    bool _is_enabled { true };
    metatype _type_info;
    std::vector<std::unique_ptr<property>> _properties;
    // For faster lookup
    std::unordered_map<std::string, property&> _properties_lt;
};
