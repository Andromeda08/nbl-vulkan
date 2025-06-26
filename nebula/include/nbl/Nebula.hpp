#pragma once

#include <memory>

namespace nbl
{
    class App;
    class VulkanRHI;

    extern std::unique_ptr<App> gApp;
    extern VulkanRHI*           gRHI;
}