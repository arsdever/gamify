#include <common/logging.hpp>
#include <project/component_interface/component.hpp>
#include <project/game_object.hpp>
#include <project/project_manager.hpp>

#include "game_context.hpp"

inline logger log() { return get_logger("game_context"); }

void game_context::initialize()
{
    log()->info("Initializing game context");

    project_manager::initialize();

    log()->info("Game context initialized successfully");
}
