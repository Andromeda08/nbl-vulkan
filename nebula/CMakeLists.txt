add_executable(Nebula
    src/Nebula.cpp
    src/wsi/Window.cpp include/nbl/wsi/Window.hpp
    src/Util.hpp
    include/nbl/app/App.hpp
    src/app/App.cpp
    include/nbl/Nebula.hpp
)

target_link_libraries(Nebula PUBLIC
    nbl_vulkan
    glfw
)

target_include_directories(Nebula PUBLIC
    ./include/nbl
    src
    ${PROJECT_SOURCE_DIR}/ext/glfw/include
    ${PROJECT_SOURCE_DIR}/nbl-vulkan/include
)

target_compile_definitions(Nebula PUBLIC
    GLFW_INCLUDE_VULKAN
)