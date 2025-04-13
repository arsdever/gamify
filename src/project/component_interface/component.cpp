#include "project/component_interface/component.hpp"

#include "common/logging.hpp"
#include "project/component_interface/component_registry.hpp"
#include "project/components/transform.hpp"
#include "project/game_object.hpp"
#include "project/project_manager.hpp"

namespace
{
logger log() { return get_logger("component"); }
} // namespace

component::component(std::string_view type_name, game_object& obj)
    : _game_object(obj)
{
    set_name(std::string(type_name));
    _type_info = component_registry::get_type(type_name);
}

game_object& component::get_game_object() const { return _game_object; }

std::shared_ptr<components::transform> component::get_transform() const
{
    return _game_object.get()
        .get_transform()
        .shared_from_this<components::transform>();
}

std::shared_ptr<component> component::get(std::string_view type_name)
{
    if (type_name == components::transform::type_name)
    {
        return get_transform();
    }

    return _game_object.get().try_get(type_name);
}

void component::set_enabled(bool active) { _is_enabled = active; }

bool component::is_enabled() const { return _is_enabled; }

bool component::set_property_value(std::string_view name,
                                   trivial_types::variant_t value)
{
    if (name == "is_enabled")
    {
        set_enabled(std::get<bool>(value));
        return true;
    }
    return false;
}

void component::for_each_property(const property_visitor_type& visitor) const
{
    visitor("is_enabled", "Is Enabled", _is_enabled);
}

std::string component::name() const { return _type_info.name; }

void component::init()
{
    if (!is_enabled())
        return;
    on_init();
}

void component::update()
{
    if (!is_enabled())
        return;
    on_update();
}

void component::deinit()
{
    if (!is_enabled())
        return;
    on_deinit();
}

bool component::can_cast(const metatype& to_type) const
{
    return _type_info.can_cast(to_type);
}

bool component::can_cast(std::string_view type_name) const
{
    return can_cast(component_registry::get_type(type_name));
}

void component::on_init() { }

void component::on_update() { }

void component::on_deinit() { }
