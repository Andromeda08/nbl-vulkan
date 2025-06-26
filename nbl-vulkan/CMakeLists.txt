add_library(nbl_vulkan
    src/vma_IMPL.cpp

    include/nbl/Common.hpp
    include/nbl/Frame.hpp
    include/nbl/IWindow.hpp

    src/Extensions.cpp          src/Extensions.hpp
    src/Util.cpp                src/Util.hpp

    src/Buffer.cpp              include/nbl/Buffer.hpp
    src/CommandQueue.cpp        include/nbl/CommandQueue.hpp
    src/Device.cpp              include/nbl/Device.hpp
    src/Descriptor.cpp          include/nbl/Descriptor.hpp
    src/Image.cpp               include/nbl/Image.hpp
    src/Swapchain.cpp           include/nbl/Swapchain.hpp
    src/VulkanRHI.cpp           include/nbl/VulkanRHI.hpp
)

target_link_libraries(nbl_vulkan PUBLIC
    fmt::fmt
    nblReflect
    GPUOpen::VulkanMemoryAllocator
    ${Vulkan_LIBRARIES}
)

target_include_directories(nbl_vulkan PUBLIC
    ./ext/fmt/include
    ./ext/nbl-reflect/include
    ./ext/vma/include
    ./include/nbl
    ${Vulkan_INCLUDE_DIRS}
)

target_compile_definitions(nbl_vulkan PUBLIC
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
)