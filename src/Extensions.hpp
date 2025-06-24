#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace nbl
{
    class VulkanDeviceExtension
    {
    public:
        /**
         * @param physicalDevice (optional) for evaluating support
         * @returns List of RHI supported Device Extensions with device support evaluation when given a physicalDevice.
         */
        static std::vector<std::unique_ptr<VulkanDeviceExtension>> getRHIDeviceExtensions(std::optional<vk::PhysicalDevice> physicalDevice = std::nullopt);

        explicit VulkanDeviceExtension(
            const char*                  extensionName,
            bool                         requested = true);

        VulkanDeviceExtension(
            const char*                  extensionName,
            const std::function<void()>& structInitFn,
            bool                         requested = true);

        virtual ~VulkanDeviceExtension() = default;

        void setRequested() noexcept { mIsRequested = true; }
        void setSupported() noexcept { mIsSupported = true; }
        void setEnabled  () noexcept { mIsEnabled   = true; }

        void postSupportCheck();

        void preCreateDevice(vk::DeviceCreateInfo& deviceCreateInfo) const;

        const char* getExtensionName() const noexcept { return mExtensionName;                    }
        bool        isRequested     () const noexcept { return mIsRequested;                      }
        bool        isSupported     () const noexcept { return mIsRequested;                      }
        bool        shouldActivate  () const noexcept { return mIsRequested     and mIsSupported; }
        bool        isActive        () const noexcept { return shouldActivate() and mIsEnabled;   }

    protected:
        void* mFeatureStructPtr = nullptr;   // For pNext chaining

    private:
        static void addToNextChain(vk::DeviceCreateInfo& deviceCreateInfo, void* featureInfo);

        const char* mExtensionName       = nullptr;
        bool        mIsCoreFeatureStruct = false;       // For Vulkan1xFeatures
        bool        mIsRequested         = false;
        bool        mIsSupported         = false;
        bool        mIsEnabled           = false;

        std::function<void()> mStructInitFn = [](){};
    };
}