#include "project/project_commands.hpp"

#include "common/logging.hpp"
#include "project/game_object.hpp"
#include "project/scene.hpp"
#include "project_manager.hpp"

namespace project
{
namespace
{
logger log() { return get_logger("project"); }
std::weak_ptr<object> _selected_object;
} // namespace

void cmd_create_game_object::execute()
{
    _obj = game_object::create();
    scene::get_active_scene()->add_root_object(_obj);
    log()->info("Game object {} ({}) was added to the scene ({})",
                _obj->get_name(),
                _obj->id().id,
                scene::get_active_scene()->id().id);
    _selected_object = _obj;
}

std::shared_ptr<game_object> cmd_create_game_object::get_object() const
{
    return _obj;
}

// cmd_destroy_game_object::cmd_destroy_game_object(
//     std::shared_ptr<game_object> obj)
//     : _obj(obj)
// {
// }

// void cmd_destroy_game_object::execute() { _obj->destroy(); }

void cmd_rename_object::execute()
{
    if (auto obj = _selected_object.lock(); obj != nullptr)
    {
        auto old_name = obj->get_name();
        obj->set_name(get<0>());
        log()->info("Object renamed ({}): {} -> {}",
                    obj->id().id,
                    old_name,
                    obj->get_name());
    }
    else
    {
        log()->info("No object selected");
    }
}

void cmd_select_object::execute()
{
    if (get<0>() == 0)
    {
        _selected_object = {};
        log()->info("Unselecting the object");
    }
    else
    {
        auto obj = project_manager::get_object_by_id(uid(get<0>()));
        if (obj == nullptr)
        {
            log()->info("Object with id {} not found", get<0>());
            return;
        }

        _selected_object = obj;
        log()->info("Selected object {} ({})", obj->get_name(), obj->id().id);
    }
}

void cmd_print_selected_object::execute()
{
    if (auto obj = _selected_object.lock(); obj != nullptr)
    {
        log()->info("Selected object {} ({})", obj->get_name(), obj->id().id);
    }
}

void cmd_list_objects::execute()
{
    log()->info("List of objects:");
    project_manager::instance().for_each_object([](std::shared_ptr<object>& obj)
    { log()->info("  {} ({})", obj->get_name(), obj->id().id); });
}

std::shared_ptr<object> selected_object() { return _selected_object.lock(); }
} // namespace project
