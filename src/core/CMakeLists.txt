add_library(
  ${PROJECT}_core STATIC
  algo.hpp
  command_dispatcher.cpp
  command_dispatcher.hpp
  command.hpp
  game_clock.hpp
  game_clock.cpp
  input_system.hpp
  input_system.cpp
  library.hpp
  settings.hpp
  settings.cpp
  timer.hpp
  timer.cpp
  viewport.hpp
  viewport.cpp
  window_events.hpp
  window_events.cpp
  window.hpp
  window.cpp
  inputs/binding.hpp
  inputs/gamepad.hpp
  inputs/gamepad.cpp)
add_library(${PROJECT}::core ALIAS ${PROJECT}_core)

if(WIN32)
  target_sources(${PROJECT}_core PRIVATE impl/library_win.cpp)
endif()

target_link_libraries(${PROJECT}_core PUBLIC ${PROJECT}::common)
target_precompile_headers(${PROJECT}_core REUSE_FROM ${PROJECT}::common)

macro(enable_asset_type)
  if(NOT ${PROJECT}_asset_loaders)
    add_custom_target(${PROJECT}_asset_loaders)
  endif()

  foreach(asset_type ${ARGN})
    string(TOLOWER ${asset_type} asset_type_lower)
    string(TOUPPER ${asset_type} asset_type_upper)
    message(STATUS "Enabling ${asset_type_upper} support")
    set(${PROJECT}_SUPPORT_${asset_type_upper} ON)
    target_link_libraries(${PROJECT}_core
                          PRIVATE ${PROJECT}::asset::${asset_type_lower})
    add_dependencies(${PROJECT}_asset_loaders
                     ${PROJECT}::asset::${asset_type_lower})
  endforeach()
endmacro()
