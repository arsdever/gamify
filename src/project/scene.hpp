#pragma once

#include "project/project_fwd.hpp"

#include "common/event.hpp"
#include "project/object.hpp"

class scene : public object
{
public:
    static std::shared_ptr<scene> create();
    static std::shared_ptr<scene> load(std::string_view path);
    static std::shared_ptr<scene> get_active_scene();

    void add_root_object(std::shared_ptr<game_object> object);

    void visit_root_objects(
        std::function<void(std::shared_ptr<game_object>&)> visitor);
    void visit_root_objects(
        std::function<void(const std::shared_ptr<game_object>&)> visitor) const;

    size_t get_root_object_count() const;
    std::shared_ptr<game_object> get_root_object(size_t index) const;
    std::shared_ptr<game_object>
    get_root_object_by_name(std::string_view name) const;

    size_t get_root_object_index(std::shared_ptr<game_object> object) const;

    void save(std::string_view path);
    void unload();

    event<bool()> save_request;
    event<void(size_t, std::shared_ptr<game_object>)> root_object_added;
    static event<void()> active_scene_changed;

private:
    scene();

    std::vector<std::shared_ptr<game_object>> _root_objects;
    static std::shared_ptr<scene> _active_scene;

    friend class project_manager;
};
