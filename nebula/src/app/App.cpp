#include "app/App.hpp"

namespace nbl
{
    App::App(const AppCreateInfo& createInfo)
    {
        mWindow = wsi::Window::createWindow(createInfo.windowInfo);

        mRHI = VulkanRHI::createVulkanRHI({
            .pWindow       = mWindow.get(),
            .configuration = createInfo.rhiInfo,
        });
    }

    void App::run()
    {

    }
}