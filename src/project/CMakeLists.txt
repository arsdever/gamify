set(HEADERS project_manager.hpp)
set(INTERFACE_HEADERS
    game_object.hpp
    object.hpp
    project_commands.hpp
    project_commands.cpp
    property.hpp
    property.cpp
    scene.hpp
    serialization_utilities.hpp
    serializer.hpp
    serializer_json.hpp)
set(SOURCES game_object.cpp project_manager.cpp object.cpp scene.cpp)

foreach(interface_header ${INTERFACE_HEADERS})
  get_filename_component(dir ${interface_header} DIRECTORY)
  message(
    STATUS ${CMAKE_CURRENT_INSTALL_DIR}/include/project/${interface_header})
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_INSTALL_DIR}/include/project/${interface_header}
           PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
            ${CMAKE_CURRENT_INSTALL_DIR}/include/project/${dir}
    COMMAND
      ${CMAKE_COMMAND} -E create_symlink
      ${CMAKE_CURRENT_INSTALL_DIR}/${interface_header}
      ${CMAKE_CURRENT_INSTALL_DIR}/include/project/${interface_header})
endforeach()

add_library(${PROJECT}_project STATIC ${INTERFACE_HEADERS} ${HEADERS}
                                      ${SOURCES})
add_library(${PROJECT}::project ALIAS ${PROJECT}_project)
target_link_libraries(
  ${PROJECT}_project
  PUBLIC ${PROJECT}::common
  PRIVATE ${PROJECT}::components)
target_include_directories(${PROJECT}_project PUBLIC ${PROJECT_SRC_DIR})

add_subdirectory(component_interface)
add_subdirectory(components)
