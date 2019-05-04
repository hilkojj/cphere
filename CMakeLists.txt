cmake_minimum_required(VERSION 3.0)

project(game)
set(EXECUTABLE_OUTPUT_PATH "bin")

file(GLOB_RECURSE source source/*)
add_executable(game ${source})
set_target_properties(game PROPERTIES LINKER_LANGUAGE CXX)

set_property(TARGET game PROPERTY CXX_STANDARD 17)
set_property(TARGET game PROPERTY CXX_STANDARD_REQUIRED ON)

# FastNoise repo:
add_library(FastNoise ./FastNoise/FastNoise.h ./FastNoise/FastNoise.cpp)
target_include_directories(FastNoise PUBLIC ./FastNoise/)

# gameutils repo:
add_subdirectory(gu/)

# -no-pie is used to get 'application/x-application' as mime type instead of 'application/x-sharedlib'
set(CMAKE_CXX_FLAGS  "-no-pie -O3")

target_link_libraries(game gameutils FastNoise)

