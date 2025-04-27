add_library(${PROJECT}_tools_profiler STATIC profiler_renderer.cpp profiler_renderer.hpp profiler.cpp profiler.hpp)
add_library(${PROJECT}::tools::profiler ALIAS ${PROJECT}_tools_profiler)

target_precompile_headers(${PROJECT}_tools_profiler REUSE_FROM
  ${PROJECT}::common)
target_link_libraries(
  ${PROJECT}_tools_profiler PUBLIC ${PROJECT}::lib ${PROJECT}::graphics
  ${PROJECT}::common)
