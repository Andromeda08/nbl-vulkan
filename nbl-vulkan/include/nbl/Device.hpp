#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include "Common.hpp"
#include "Util.hpp"

namespace nbl
{
    class VulkanDeviceExtension;

    struct DeviceCreateInfo
    {
        vk::Instance instance;
    };

    template <class T>
    struct VulkanNameObjectInfo
    {
        std::string debugName;
        T           handle;
    };

    class Device
    {
    public:
        nbl_DISABLE_COPY(Device);
        nbl_CI_CTOR(Device, DeviceCreateInfo);

        ~Device();

        void waitIdle() const;

        vk::Device          getHandle()            const { return mDevice;                  }
        const VmaAllocator& getAllocator()         const { return mAllocator;               }
        vk::PhysicalDevice  getPhysicalDevice()    const { return mPhysicalDevice;          }

        Queue*              getGraphicsQueue()     const { return mGraphicsQueue.get();     }
        Queue*              getAsyncComputeQueue() const { return mAsyncComputeQueue.get(); }

        /**
         * Set the debug name for a Vulkan object.
         * @tparam T Vulkan object type
         */
        template <class T>
        void nameObject(const VulkanNameObjectInfo<T>& nameObjectInfo) const;

    private:
        void selectPhysicalDevice();
        void createDevice();
        void createAllocator();

        std::unique_ptr<Queue> createQueue(const QueueCreateInfo& createInfo) const;

        vk::Instance                                        mInstance;

        vk::PhysicalDevice                                  mPhysicalDevice;
        vk::PhysicalDeviceProperties                        mPhysicalDeviceProperties;
        std::string                                         mDeviceName;

        vk::Device                                          mDevice;
        std::vector<std::unique_ptr<VulkanDeviceExtension>> mDeviceExtensions;
        std::vector<const char*>                            mDeviceExtensionNames;

        std::unique_ptr<Queue>                              mGraphicsQueue;
        std::unique_ptr<Queue>                              mAsyncComputeQueue;

        VmaAllocator                                        mAllocator {};
    };

    template<class T>
    void Device::nameObject(const VulkanNameObjectInfo<T>& nameObjectInfo) const
    {
        const std::string name = nameObjectInfo.debugName.empty() ? "Unknown" :  nameObjectInfo.debugName;
        const auto nameInfo = vk::DebugUtilsObjectNameInfoEXT()
            .setPObjectName(name.c_str())
            // ReSharper disable once CppFunctionalStyleCast
            // ReSharper disable once CppDependentTypeWithoutTypenameKeyword
            .setObjectHandle(uint64_t(static_cast<T::CType>(nameObjectInfo.handle)))
            .setObjectType(nameObjectInfo.handle.objectType);

        mDevice.setDebugUtilsObjectNameEXT(nameInfo);
    }
}