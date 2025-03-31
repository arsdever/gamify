set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_library(
	${PROJECT}_ui_widgets
	STATIC
	color_widget.hpp
	color_widget.cpp
	texture_widget.hpp
	texture_widget.cpp
	vec3_widget.hpp
	vec3_widget.cpp
)
add_library(${PROJECT}::ui::widgets ALIAS ${PROJECT}_ui_widgets)

target_link_libraries(${PROJECT}_ui_widgets PRIVATE
	${PROJECT}::lib
	Qt::Widgets
	Qt::Core
	Qt::Gui
)
