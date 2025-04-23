#include "project/property.hpp"

property::property() = default;

property::property(std::string_view name)
    : _name(name)
{
}

property::property(std::string_view name,
                   std::string_view display_name,
                   std::string_view type,
                   std::string_view description,
                   std::any default_value,
                   //    std::string_view category,
                   bool read_only,
                   bool hidden,
                   bool advanced,
                   bool required,
                   bool deprecated)
    : _name(name)
    , _display_name(display_name)
    , _type(type)
    , _description(description)
    , _default_value(default_value)
    // , _category(category)
    , _is_read_only(read_only)
    , _is_hidden(hidden)
    , _is_advanced(advanced)
    , _is_required(required)
    , _is_deprecated(deprecated)
{
}

property::property(const property& other)
    : _name(other._name)
    , _display_name(other._display_name)
    , _type(other._type)
    , _description(other._description)
    , _value(other._value)
    , _default_value(other._default_value)
    // , _category(other._category)
    , _is_read_only(other._is_read_only)
    , _is_hidden(other._is_hidden)
    , _is_advanced(other._is_advanced)
    , _is_required(other._is_required)
    , _is_deprecated(other._is_deprecated)
{
}

property::property(property&& other)
    : _name(std::move(other._name))
    , _display_name(std::move(other._display_name))
    , _type(std::move(other._type))
    , _description(std::move(other._description))
    , _value(std::move(other._value))
    , _default_value(std::move(other._default_value))
    // , _category(std::move(other._category))
    , _is_read_only(other._is_read_only)
    , _is_hidden(other._is_hidden)
    , _is_advanced(other._is_advanced)
    , _is_required(other._is_required)
    , _is_deprecated(other._is_deprecated)
{
}

property& property::operator=(const property& other)
{
    if (this != &other)
    {
        _name = other._name;
        _display_name = other._display_name;
        _type = other._type;
        _description = other._description;
        _value = other._value;
        _default_value = other._default_value;
        // _category = other._category;
        _is_read_only = other._is_read_only;
        _is_hidden = other._is_hidden;
        _is_advanced = other._is_advanced;
        _is_required = other._is_required;
        _is_deprecated = other._is_deprecated;
    }
    return *this;
}

property& property::operator=(property&& other)
{
    if (this != &other)
    {
        _name = std::move(other._name);
        _display_name = std::move(other._display_name);
        _type = std::move(other._type);
        _description = std::move(other._description);
        _value = std::move(other._value);
        _default_value = std::move(other._default_value);
        // _category = std::move(other._category);
        _is_read_only = other._is_read_only;
        _is_hidden = other._is_hidden;
        _is_advanced = other._is_advanced;
        _is_required = other._is_required;
        _is_deprecated = other._is_deprecated;
    }
    return *this;
}

property::~property() = default;

std::string_view property::get_name() const { return _name; }

void property::set_display_name(std::string_view display_name)
{
    _display_name = display_name;
}

std::string property::get_display_name() const { return _display_name; }

void property::set_default_value( std::any value)
{
    _default_value = std::move(value);
}

std::any property::get_default_value() const { return _default_value; }

void property::set_value(std::any value)
{
    _value = std::move(value);
    value_changed();
}

std::any property::get_value() const
{
    if (_value.has_value())
    {
        return _value;
    }
    else
    {
        return _default_value;
    }
}

bool property::has_value() const { return _value.has_value(); }

void property::set_type(std::string_view type) { _type = type; }

std::string property::get_type() const { return _type; }

void property::set_description(std::string_view description)
{
    _description = description;
}

std::string property::get_description() const { return _description; }

// void property::set_category(std::string_view category)
// {
//     _category = category;
// }

// std::string property::get_category() const { return _category; }

void property::set_read_only(bool is_read_only)
{
    _is_read_only = is_read_only;
}

bool property::is_read_only() const { return _is_read_only; }

void property::set_hidden(bool is_hidden) { _is_hidden = is_hidden; }

bool property::is_hidden() const { return _is_hidden; }

void property::set_advanced(bool is_advanced) { _is_advanced = is_advanced; }

bool property::is_advanced() const { return _is_advanced; }

void property::set_required(bool is_required) { _is_required = is_required; }

bool property::is_required() const { return _is_required; }

void property::set_deprecated(bool is_deprecated)
{
    _is_deprecated = is_deprecated;
}

bool property::is_deprecated() const { return _is_deprecated; }
