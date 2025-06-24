#pragma once

#include <vulkan/vulkan.hpp>
#include "Util.hpp"

namespace nbl
{
    class Device;
    class IWindow;

    struct VulkanRHIConfiguration
    {
        bool        validation      = false;
        uint32_t    backBufferCount = 2;
        std::string applicationName = "Unknown Application";
        std::string engineName      = "nbl::VulkanRHI";
    };

    struct VulkanRHICreateInfo
    {
        IWindow*                pWindow       = nullptr;
        VulkanRHIConfiguration  configuration = {};
    };

    /**
     * Vulkan Rendering Hardware Interface
     * [Only one active instance can exist at a time]
     */
    class VulkanRHI
    {
    public:
        nbl_DISABLE_COPY(VulkanRHI);
        nbl_CI_CTOR(VulkanRHI, VulkanRHICreateInfo);

        Device* getDevice() const { return mDevice.get(); }

    private:
        void createInstance();

        static bool                     sExists;

        IWindow*                        mWindow = nullptr;
        const VulkanRHIConfiguration    mConfig = {};

        vk::Instance                    mInstance;
        std::vector<const char*>        mInstanceLayers;
        std::vector<const char*>        mInstanceExtensions;

        std::unique_ptr<Device>         mDevice;
    };
}