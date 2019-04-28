cmake_minimum_required(VERSION 3.0)

project(game)
set(EXECUTABLE_OUTPUT_PATH "bin")

file(GLOB_RECURSE source source/*)
add_executable(game ${source})
set_target_properties(game PROPERTIES LINKER_LANGUAGE CXX)

# gameutils repo:
add_subdirectory(gu/)

# -no-pie is used to get 'application/x-application' as mime type instead of 'application/x-sharedlib'
set(CMAKE_CXX_FLAGS  "-no-pie")

target_link_libraries(game gameutils)

