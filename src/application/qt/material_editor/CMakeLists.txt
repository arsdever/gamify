set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(
	${PROJECT}_apps_material_editor
	material_editor.cpp
	material_property_ui_builder.cpp
	material_property_ui_builder.hpp
	resources.qrc
)

target_link_libraries(${PROJECT}_apps_material_editor PRIVATE
	${PROJECT}::lib
	${PROJECT}::tools
	Qt::Widgets
	Qt::Core
	Qt::Gui
)
find_program(Qt6 QUIET Qt::windeployqt)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	find_program(TOOL_WINDEPLOYQT NAMES windeployqt.debug.bat)
else()
	find_program(TOOL_WINDEPLOYQT NAMES windeployqt)
endif()

message(STATUS "windepolyqt found at " ${${PROJECT}_LIBRARY_DIR})
add_custom_command(
	TARGET ${PROJECT}_apps_material_editor POST_BUILD
	COMMAND ${TOOL_WINDEPLOYQT}
	ARGS
	$<TARGET_FILE:${PROJECT}_apps_material_editor> --dir ${${PROJECT}_LIBRARY_DIR}
)
