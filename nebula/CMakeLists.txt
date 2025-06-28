set("CY_HAIR_FILES" ${PROJECT_SOURCE_DIR}/ext/cy/cyCore.h ${PROJECT_SOURCE_DIR}/ext/cy/cyHairFile.h)

set("IMGUI_DIR" ${PROJECT_SOURCE_DIR}/ext/imgui)
set("IMGUI_FILES"
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
    ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui.h ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_demo.cpp ${IMGUI_DIR}/imgui_tables.cpp ${IMGUI_DIR}/imgui_widgets.cpp
)

add_executable(Nebula
    ${CY_HAIR_FILES}
    ${IMGUI_FILES}


    src/Util.hpp
    src/Nebula.cpp                          include/nbl/Nebula.hpp
    src/app/App.cpp                         include/nbl/app/App.hpp

    src/wsi/Window.cpp                      include/nbl/wsi/Window.hpp

    include/nbl/ui/UIComponent.hpp
    src/ui/UserInterface.cpp                include/nbl/ui/UserInterface.hpp

    include/nbl/hair/HairCommon.h
    src/hair/HairModel.cpp                  include/nbl/hair/HairModel.hpp
    src/hair/HairPipeline.cpp               include/nbl/hair/HairPipeline.hpp
    src/hair/HairUIComponent.cpp            include/nbl/hair/HairUIComponent.hpp

    include/nbl/camera/CameraData.hpp
    include/nbl/camera/ICamera.hpp
    src/camera/FirstPersonCamera.cpp        include/nbl/camera/FirstPersonCamera.hpp
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
    ${PROJECT_SOURCE_DIR}/ext/imgui
    ${PROJECT_SOURCE_DIR}/nbl-vulkan/include
)

target_compile_definitions(Nebula PUBLIC
    GLFW_INCLUDE_VULKAN
    -DImTextureID=ImU64
)