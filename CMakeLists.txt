cmake_minimum_required(VERSION 3.30)
project(nebula_next)

set(CMAKE_CXX_STANDARD 23)

find_package(Vulkan REQUIRED)

add_subdirectory(ext/fmt)
add_subdirectory(ext/nbl-reflect)
add_subdirectory(ext/vma)

add_library(nebula_next
    include/nbl/VulkanRHI.hpp
    src/VulkanRHI.cpp
    src/vma_IMPL.cpp
    src/Util.hpp
    src/Util.cpp
    include/nbl/IWindow.hpp
    include/nbl/Device.hpp
    src/Extensions.cpp
    src/Extensions.hpp
    src/Device.cpp
    include/nbl/Common.hpp
)

target_link_libraries(nebula_next PUBLIC
    fmt::fmt
    nblReflect
    GPUOpen::VulkanMemoryAllocator
    ${Vulkan_LIBRARIES}
)

target_include_directories(nebula_next PUBLIC
    ./ext/fmt/include
    ./ext/nbl-reflect/include
    ./ext/vma/include
    ./include/nbl
    ${Vulkan_INCLUDE_DIRS}
)

target_compile_definitions(nebula_next PUBLIC
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
)