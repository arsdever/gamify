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

    add_property({ "is_enabled",
                   "Is Enabled",
                   "Enables or disables the component",
                   bool {} })
        .set_value(true);
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

void component::set_enabled(bool active)
{
    set_property_value("is_enabled", active);
}

bool component::is_enabled() const
{
    return get_property_value<bool>("is_enabled");
}

bool component::set_property_value(std::string_view name, std::any value)
{
    auto& prop = get_property(name);
    prop.set_value(std::move(value));
    return false;
}

void component::for_each_property(
    const property_const_visitor_type& visitor) const
{
    for (const auto& prop : _properties)
    {
        if (!visitor(*prop))
        {
            break;
        }
    }
}

void component::for_each_property(const property_visitor_type& visitor)
{
    for (auto& prop : _properties)
    {
        if (!visitor(*prop))
        {
            break;
        }
    }
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

property& component::add_property(property&& prop)
{
    auto* p =
        _properties.emplace_back(std::make_unique<property>(std::move(prop)))
            .get();
    _properties_lt.emplace(std::string(p->get_name()), *p);
    return *p;
}

const property& component::get_property(std::string_view name) const
{
    auto* prop = try_get_property(name);
    if (prop == nullptr)
    {
        throw std::runtime_error("Property not found");
    }

    return *prop;
}

const property* component::try_get_property(std::string_view name) const
{
    auto it = _properties_lt.find(std::string(name));
    if (it != _properties_lt.end())
    {
        return &it->second;
    }

    return nullptr;
}

property& component::get_property(std::string_view name)
{
    auto* prop = try_get_property(name);
    if (prop == nullptr)
    {
        throw std::runtime_error("Property not found");
    }

    return *prop;
}

property* component::try_get_property(std::string_view name)
{
    auto it = _properties_lt.find(std::string(name));
    if (it != _properties_lt.end())
    {
        return &it->second;
    }

    return nullptr;
}

void component::on_init() { }

void component::on_update() { }

void component::on_deinit() { }
