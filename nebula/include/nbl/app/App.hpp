#pragma once

#include <memory>
#include <nbl/VulkanRHI.hpp>
#include <wsi/Window.hpp>

namespace nbl
{
    struct AppCreateInfo
    {
        wsi::WindowCreateInfo   windowInfo    = {};
        VulkanRHIConfiguration  rhiInfo       = {};
        bool                    enableUI      = true;
        bool                    renderGraph   = true;
        bool                    exampleScene  = true;
    };

    class App
    {
    public:
        nbl_CI_CTOR(App, AppCreateInfo);

        virtual ~App() = default;

        virtual void run();

    private:
        std::unique_ptr<wsi::Window> mWindow;
        std::unique_ptr<VulkanRHI>   mRHI;
    };
}