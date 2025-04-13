#pragma once

#include <nlohmann/json_fwd.hpp>

#include "project/project_fwd.hpp"

#include "project/object.hpp"

class game_object : public object
{
public:
    static std::shared_ptr<game_object> create();

    components::transform& get_transform() const;

    bool has_parent() const;
    std::shared_ptr<game_object> get_parent() const;

    std::shared_ptr<game_object> get_child_at(size_t index) const;
    std::shared_ptr<game_object> get_child_by_name(std::string_view name) const;
    void add_child(std::shared_ptr<game_object> child);
    void remove_child(std::shared_ptr<game_object> child);
    bool visit_children(
        std::function<bool(const std::shared_ptr<game_object>&)> visitor) const;

    size_t get_child_index(std::shared_ptr<game_object> child) const;
    size_t get_child_count() const;

    component& add(std::string_view class_name);
    void remove(std::shared_ptr<component> cmp);

    template <typename T>
        requires(std::is_base_of<component, T>::value)
    T& add()
    {
        return static_cast<T&>(add(T::type_name));
    }

    component& get(std::string_view class_name) const;
    std::shared_ptr<component> try_get(std::string_view class_name) const;

    template <typename T>
        requires(std::is_base_of<component, T>::value)
    T& get() const
    {
        return static_cast<T&>(get(T::type_name));
    }

    template <typename T>
        requires(std::is_base_of<component, T>::value)
    std::shared_ptr<T> try_get() const
    {
        return std::static_pointer_cast<T>(try_get(T::type_name));
    }

    void visit_components(std::function<bool(component&)> visitor) const;

    void set_active(bool active = true);
    bool is_active() const;

    void init();
    void update();
    void draw_gizmos();
    void deinit();

protected:
    void set_parent(std::shared_ptr<game_object> parent);

private:
    friend class project_manager;
    game_object();

private:
    bool _is_active { true };
    std::weak_ptr<game_object> _parent {};
    std::vector<std::shared_ptr<game_object>> _children {};
};
