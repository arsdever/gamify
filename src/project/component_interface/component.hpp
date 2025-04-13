#pragma once

#include "project/project_fwd.hpp"

#include "project/component_interface/metatype.hpp"
#include "project/definitions.hpp"
#include "project/game_object.hpp"
#include "project/object.hpp"

class component : public object
{
public:
    using property_visitor_type = std::function<bool(
        std::string_view, std::string_view, const trivial_types::variant_t&)>;

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

    virtual bool set_property_value(std::string_view name,
                                    trivial_types::variant_t value);

    virtual void for_each_property(const property_visitor_type& visitor) const;
    std::string name() const;

    void init();
    void update();
    void deinit();

    bool can_cast(const metatype& to_type) const;
    bool can_cast(std::string_view type_name) const;

protected:
    component(std::string_view name, game_object& obj);

    virtual void on_init();
    virtual void on_update();
    virtual void on_deinit();

    std::reference_wrapper<game_object> _game_object;

private:
    bool _is_enabled { true };
    metatype _type_info;
};
