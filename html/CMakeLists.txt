cmake_minimum_required(VERSION 3.0)

project(game)
set(EXECUTABLE_OUTPUT_PATH "out")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE source ../source/*)
add_executable(game ${source})
set_target_properties(game PROPERTIES LINKER_LANGUAGE CXX)

set_property(TARGET game PROPERTY CXX_STANDARD 17)
set_property(TARGET game PROPERTY CXX_STANDARD_REQUIRED ON)

# gameutils repo:
add_subdirectory(../gu/library/ ./bin/gu)

# FastNoise repo:
add_library(FastNoise ../FastNoise/FastNoise.h ../FastNoise/FastNoise.cpp)
target_include_directories(FastNoise PUBLIC ../FastNoise/)

set_property(TARGET FastNoise PROPERTY CXX_STANDARD 17)
set_property(TARGET FastNoise PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(game gameutils FastNoise)

set(CMAKE_CXX_FLAGS  "-O3")

set_target_properties(game PROPERTIES SUFFIX ".html")
set_target_properties(game PROPERTIES LINK_FLAGS "-s WASM=1 -s ASSERTIONS=1 -s ALLOW_MEMORY_GROWTH=1 -s USE_GLFW=3 -s USE_WEBGL2=1 -s BINARYEN_METHOD='native-wasm' -s BINARYEN_TRAP_MODE='clamp' --preload-file ../assets --shell-file index.html --no-heap-copy")

# creates the "out" directory:
add_custom_command(
    TARGET game PRE_LINK
    COMMAND ${CMAKE_COMMAND} -E make_directory out)

# removes the old "out" directory:
add_custom_target(
    removeOut
    COMMAND ${CMAKE_COMMAND} -E remove_directory out)
add_dependencies(game removeOut)
