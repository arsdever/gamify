#pragma once

#include <type_traits>

#include "common/event.hpp"

template <typename T, typename... Args>
struct is_flag_set
{
    static constexpr bool value = (std::is_same_v<T, Args> || ...);
};

template <typename T, typename... Args>
inline constexpr bool is_flag_set_v = is_flag_set<T, Args...>::value;

class property
{
public:
    struct read_only;
    struct hidden;
    struct advanced;
    struct required;
    struct deprecated;

    template <typename T>
    struct type;

public:
    property();
    property(std::string_view name);

    template <typename V, typename... Attributes>
    property(std::string_view name,
             std::string_view display_name,
             std::string_view description,
             V&& default_value)
        : property(name,
                   display_name,
                   typeid(V).name(),
                   description,
                   std::forward<V>(default_value),
                   //  std::string_view category,
                   is_flag_set_v<read_only, Attributes...>,
                   is_flag_set_v<hidden, Attributes...>,
                   is_flag_set_v<advanced, Attributes...>,
                   is_flag_set_v<required, Attributes...>,
                   is_flag_set_v<deprecated, Attributes...>)
    {
    }

    template <typename V, typename... Attributes>
    property(std::string_view name,
             std::string_view display_name,
             std::string_view description,
             type<V> = {})
        : property(name,
                   display_name,
                   typeid(V).name(),
                   description,
                   V {},
                   //  std::string_view category,
                   is_flag_set_v<read_only, Attributes...>,
                   is_flag_set_v<hidden, Attributes...>,
                   is_flag_set_v<advanced, Attributes...>,
                   is_flag_set_v<required, Attributes...>,
                   is_flag_set_v<deprecated, Attributes...>)
    {
    }

    property(std::string_view name,
             std::string_view display_name,
             std::string_view type,
             std::string_view description,
             // We will anyway copy the value, so either it's been moved and we
             // move forward or we just get the copy and move forward
             std::any default_value,
             //  std::string_view category,
             bool read_only,
             bool hidden,
             bool advanced,
             bool required,
             bool deprecated);
    property(const property&);
    property(property&&);
    property& operator=(const property&);
    property& operator=(property&&);

    ~property();

    std::string_view get_name() const;

    void set_display_name(std::string_view display_name);
    std::string get_display_name() const;

    void set_default_value(std::any value);
    std::any get_default_value() const;
    template <typename T>
    T get_default_value() const
    {
        return std::any_cast<T>(get_default_value());
    }

    // Either the type is already moved, or we just get the copy and later move
    // into the storage
    void set_value(std::any value);
    template <typename T>
    void set_value(T&& value)
    {
        set_value(std::any(std::forward<T>(value)));
    }

    inline property& operator=(std::any value)
    {
        set_value(std::move(value));
        return *this;
    }

    template <typename T>
    property& operator=(T&& value)
    {
        set_value(std::forward<T>(value));
        return *this;
    }

    std::any get_value() const;

    template <typename T>
    T get_value() const
    {
        return std::any_cast<T>(get_value());
    }

    bool has_value() const;

    void set_type(std::string_view type);
    std::string get_type() const;

    void set_description(std::string_view description);
    std::string get_description() const;

    // void set_category(std::string_view category);
    // std::string get_category() const;

    void set_read_only(bool read_only);
    bool is_read_only() const;

    void set_hidden(bool hidden);
    bool is_hidden() const;

    void set_advanced(bool advanced);
    bool is_advanced() const;

    void set_required(bool required);
    bool is_required() const;

    void set_deprecated(bool deprecated);
    bool is_deprecated() const;

    event<void()> value_changed;

private:
    std::string _name;
    std::string _display_name;
    std::string _type;
    std::string _description;
    std::any _value;
    std::any _default_value;
    // std::string _category;
    bool _is_read_only = false;
    bool _is_hidden = false;
    bool _is_advanced = false;
    bool _is_required = false;
    bool _is_deprecated = false;
};

namespace std
{
template <>
struct hash<property>
{
    size_t operator()(const property& prop) const
    {
        return std::hash<std::string_view>()(prop.get_name()) ^
               std::hash<std::string_view>()(prop.get_type());
    }
};
} // namespace std
