
cmake_minimum_required(VERSION 2.8)

project(glfwdiag C)

set(GLFW_INSTALL OFF CACHE STRING "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE STRING "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE STRING "" FORCE)

add_subdirectory(deps/glfw)
include_directories(deps/glfw/include)
include_directories(deps/glfw/deps)

add_subdirectory(src)

