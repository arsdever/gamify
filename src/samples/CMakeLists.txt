add_executable(${PROJECT}_window_sample window.cpp)
target_link_libraries(${PROJECT}_window_sample PUBLIC ${PROJECT}::lib)

add_executable(${PROJECT}_viewport_sample viewport.cpp)
target_link_libraries(${PROJECT}_viewport_sample PUBLIC ${PROJECT}::lib)

add_executable(${PROJECT}_multiple_viewport_sample multiple_viewports.cpp)
target_link_libraries(${PROJECT}_multiple_viewport_sample
                      PUBLIC ${PROJECT}::lib)

add_executable(${PROJECT}_scene_load scene_load.cpp)
target_link_libraries(${PROJECT}_scene_load PUBLIC ${PROJECT}::lib)

add_executable(${PROJECT}_sol2 sol2.cpp)
target_link_libraries(${PROJECT}_sol2 PUBLIC sol2 lua54)
