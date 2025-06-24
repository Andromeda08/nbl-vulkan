#include "VulkanRHI.hpp"

#include "Device.hpp"
#include "IWindow.hpp"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;

namespace nbl
{
    bool VulkanRHI::sExists = false;

    VulkanRHI::VulkanRHI(const VulkanRHICreateInfo& createInfo)
    : mWindow(createInfo.pWindow)
    , mConfig(createInfo.configuration)
    {
        if (sExists)
        {
            throw RHIError("There can only be one active instance of VulkanRHI.");
        }
        sExists = true;

        const vk::detail::DynamicLoader dynamicLoader;
        const auto vkGetInstanceProcAddr = dynamicLoader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

        createInstance();

        mDevice = Device::createDevice({
            .instance = mInstance,
        });
        VULKAN_HPP_DEFAULT_DISPATCHER.init(mDevice->getHandle());
    }

    void VulkanRHI::createInstance()
    {
        const auto applicationInfo = vk::ApplicationInfo()
            .setApiVersion(VK_API_VERSION_1_4)
            .setPApplicationName(mConfig.applicationName.c_str())
            .setPEngineName(mConfig.engineName.c_str());

        std::vector<const char*> layers {};
        if (mConfig.validation)
        {
            layers.push_back("VK_LAYER_KHRONOS_validation");
        }

        const std::vector driverLayers = vk::enumerateInstanceLayerProperties();
        mInstanceLayers = getSupportedLayers(layers, driverLayers);

        std::vector<const char*> extensions {};
        if (mWindow)
        {
            extensions.append_range(mWindow->getVulkanInstanceExtensions());
        }

        const std::vector driverExtensions = vk::enumerateInstanceExtensionProperties();
        mInstanceExtensions = getSupportedExtensions(extensions, driverExtensions);

        const auto instanceCreateInfo = vk::InstanceCreateInfo()
            .setEnabledExtensionCount(mInstanceExtensions.size())
            .setPpEnabledExtensionNames(mInstanceExtensions.data())
            .setEnabledLayerCount(mInstanceLayers.size())
            .setPpEnabledLayerNames(mInstanceLayers.data())
            .setPApplicationInfo(&applicationInfo);

        nbl_VK_TRY(mInstance = vk::createInstance(instanceCreateInfo);)
    }
}
