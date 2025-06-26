set("CY_HAIR_FILES" ${PROJECT_SOURCE_DIR}/ext/cy/cyCore.h ${PROJECT_SOURCE_DIR}/ext/cy/cyHairFile.h)

add_executable(Nebula
    ${CY_HAIR_FILES}

    src/Nebula.cpp
    src/wsi/Window.cpp include/nbl/wsi/Window.hpp
    src/Util.hpp
    include/nbl/app/App.hpp
    src/app/App.cpp
    include/nbl/Nebula.hpp

    include/nbl/hair/HairModel.hpp
    include/nbl/hair/HairObject.hpp
    include/nbl/hair/HairCommon.h
    include/nbl/hair/HairUIComponent.hpp
    src/hair/HairModel.cpp
    include/nbl/hair/HairRenderer.hpp
    include/nbl/hair/HairRendererApp.hpp
)

target_link_libraries(Nebula PUBLIC
    nbl_vulkan
    glfw
    glm::glm
)

target_include_directories(Nebula PUBLIC
    ./include/nbl
    src
    ${PROJECT_SOURCE_DIR}/ext/cy
    ${PROJECT_SOURCE_DIR}/ext/glfw/include
    ${PROJECT_SOURCE_DIR}/ext/glm
    ${PROJECT_SOURCE_DIR}/nbl-vulkan/include
)

target_compile_definitions(Nebula PUBLIC
    GLFW_INCLUDE_VULKAN
)