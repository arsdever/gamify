add_library(
  ${PROJECT}_renderer STATIC
  renderer_2d.hpp
  renderer_2d.cpp
  renderer_3d.hpp
  renderer_3d.cpp
  renderer.hpp
  renderer.cpp
  algorithms/polygon_to_mesh.hpp
  algorithms/polygon_to_mesh.cpp)
add_library(${PROJECT}::renderer ALIAS ${PROJECT}_renderer)

target_precompile_headers(${PROJECT}_renderer REUSE_FROM ${PROJECT}::common)
target_include_directories(
  ${PROJECT}_renderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_SRC_DIR})
target_link_libraries(${PROJECT}_renderer PRIVATE glfw glm ${PROJECT}::common
                                                  ${PROJECT}::glad)
