cmake_minimum_required(VERSION 3.17)

set(CMAKE_TOOLCHAIN_FILE F:/vcpkg/scripts/buildsystems/vcpkg.cmake)

project(sys CXX C CUDA)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

add_subdirectory(GUI)

add_subdirectory(plugin)

add_subdirectory(tools)

add_subdirectory(src)