cmake_minimum_required(VERSION 3.30)
project(nbl_vulkan)

set(CMAKE_CXX_STANDARD 23)

find_package(Vulkan REQUIRED)

add_subdirectory(ext/fmt)

add_subdirectory(ext/glm)
add_compile_definitions(GLM_FORCE_DEPTH_ZERO_TO_ONE GLM_ENABLE_EXPERIMENTAL)

add_subdirectory(ext/glfw)
add_subdirectory(ext/nbl-reflect)
add_subdirectory(ext/vma)

add_subdirectory(nbl-vulkan)
add_subdirectory(nebula)