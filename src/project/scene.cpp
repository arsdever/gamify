#include <nlohmann/json.hpp>

#include "project/scene.hpp"

#include "common/file.hpp"
#include "common/logging.hpp"
#include "project/component_interface/component.hpp"
#include "project/game_object.hpp"
#include "project/project_manager.hpp"

namespace
{
logger log() { return get_logger("project"); }
} // namespace

scene::scene() = default;

std::shared_ptr<scene> scene::get_active_scene() { return _active_scene; }

void scene::add_root_object(std::shared_ptr<game_object> object)
{
    _root_objects.push_back(object);
    root_object_added(_root_objects.size() - 1, object);
}

void scene::visit_root_objects(
    std::function<void(std::shared_ptr<game_object>&)> visitor)
{
    for (auto& object : _root_objects)
    {
        visitor(object);
    }
}

void scene::visit_root_objects(
    std::function<void(const std::shared_ptr<game_object>&)> visitor) const
{
    for (auto& object : _root_objects)
    {
        visitor(object);
    }
}

std::shared_ptr<scene> scene::create()
{
    _active_scene = std::shared_ptr<scene>(new scene());
    active_scene_changed();
    return _active_scene;
}

size_t scene::get_root_object_count() const { return _root_objects.size(); }

std::shared_ptr<game_object> scene::get_root_object(size_t index) const
{
    if (index >= _root_objects.size())
        return nullptr;
    return _root_objects[ index ];
}

std::shared_ptr<game_object>
scene::get_root_object_by_name(std::string_view name) const
{
    for (auto& object : _root_objects)
    {
        if (object->get_name() == name)
            return object;
    }
    return nullptr;
}

size_t scene::get_root_object_index(std::shared_ptr<game_object> object) const
{
    for (size_t i = 0; i < _root_objects.size(); ++i)
    {
        if (_root_objects[ i ] == object)
            return i;
    }
    return 0;
}

void scene::save(std::string_view path)
{
    std::ofstream file { std::string(path) };
    file << project_manager::serialize().dump() << std::endl;
}

std::shared_ptr<scene> scene::load(std::string_view path)
{
    if (!common::file::exists(path))
    {
        log()->error("Failed to load scene: {} does not exist", path);
        return nullptr;
    }

    try
    {
        nlohmann::json data =
            nlohmann::json::parse(common::file::read_all(path));
        _active_scene = std::shared_ptr<scene>(new scene());
        active_scene_changed();
        project_manager::deserialize(data);
    }
    catch (nlohmann::json::exception& ex)
    {
        log()->error("Failed to load scene: {}", ex.what());
        return nullptr;
    }
    return _active_scene;
}

void scene::unload()
{
    _active_scene = nullptr;
    active_scene_changed();
}

std::shared_ptr<scene> scene::_active_scene { nullptr };

event<void()> scene::active_scene_changed;
