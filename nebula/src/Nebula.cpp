#include <iostream>
#include <memory>

#include <app/App.hpp>
#include <nbl/VulkanRHI.hpp>
#include <wsi/Window.hpp>

namespace nbl
{
    std::unique_ptr<App> gApp;
    VulkanRHI*           gRHI;
}

int main(int argc, char** argv)
{
    using namespace nbl;

    const std::string name = "nbl::Engine";

    try
    {
        gApp = App::createApp({
            .windowInfo = {
                .title            = name,
                .resolutionPreset =  wsi::WindowResolutionPreset::w1920_h1080,
            },
            .rhiInfo    = {
                .validation      = true,
                .backBufferCount = 2,
                .applicationName = name,
                .engineName      = name,
            },
        });
        gApp->run();
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return -1;
    }

    return 0;
}
