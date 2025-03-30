cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(PROJECT gamify)

project(${PROJECT} LANGUAGES C CXX)

set(${PROJECT}_LIBRARY_DIR ${CMAKE_BINARY_DIR}/lib)

message(STATUS "Setting default binary directory to ${RUNTIME_DIRECTORY}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${RUNTIME_DIRECTORY})

set(BUILD_DEBUG ON)

include($ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)
find_package(spdlog REQUIRED)
find_package(freetype REQUIRED)

add_subdirectory(thirdparty/prof)
add_subdirectory(thirdparty/qspdlog)

if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/thirdparty/filewatch)
  add_library(filewatch INTERFACE IMPORTED)
  target_include_directories(filewatch INTERFACE "thirdparty/filewatch")
  set(USE_FILEWATCH TRUE)
endif()

add_subdirectory(src)

if(BUILD_TESTING)
  message(STATUS "Building tests")
  set(${PROJECT}_TESTING_ENABLED ON)
endif()

if(${PROJECT}_TESTING_ENABLED)
  enable_testing()
  add_subdirectory(unittest)
  add_subdirectory(test)
endif()
