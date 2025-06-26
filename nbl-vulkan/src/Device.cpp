#include "Device.hpp"

#include "Extensions.hpp"

namespace nbl
{
    inline vk::PhysicalDeviceFeatures getBaseDeviceFeatures() noexcept
    {
        return vk::PhysicalDeviceFeatures()
            .setGeometryShader(true)
            .setTessellationShader(true)
            .setMultiDrawIndirect(true)
            .setDrawIndirectFirstInstance(true)
            .setFillModeNonSolid(true)
            .setSamplerAnisotropy(true)
            .setSampleRateShading(true)
            .setShaderInt64(true);
    }

    Device::Device(const DeviceCreateInfo& createInfo)
    : mInstance(createInfo.instance)
    {
        selectPhysicalDevice();
        createDevice();
        createAllocator();
    }

    void Device::waitIdle() const
    {
        mDevice.waitIdle();
    }

    Device::~Device()
    {
        waitIdle();
    }

    void Device::selectPhysicalDevice()
    {
        const auto physicalDevices = mInstance.enumeratePhysicalDevices();
        const auto candidate = std::ranges::find_if(physicalDevices, [&](const vk::PhysicalDevice& physicalDevice) {
            bool requirementsPassed = true;
            for (const auto& extension : VulkanDeviceExtension::getRHIDeviceExtensions(physicalDevice))
            {
                if (extension->isRequested() and !extension->isSupported())
                {
                    requirementsPassed = false;
                }
            }

            return requirementsPassed;
        });

        if (candidate == std::end(physicalDevices))
        {
            throw RHIError("Failed to find a suitable PhysicalDevice");
        }

        mPhysicalDevice = *candidate;
        mPhysicalDeviceProperties = mPhysicalDevice.getProperties();
        mDeviceName = std::string(mPhysicalDeviceProperties.deviceName.data());
    }

    void Device::createDevice()
    {
        #pragma region "Extensions"

        mDeviceExtensions = VulkanDeviceExtension::getRHIDeviceExtensions(mPhysicalDevice);
        for (auto& extension : mDeviceExtensions)
        {
            if (extension->shouldActivate())
            {
                extension->postSupportCheck();
                if (extension->isExtension())
                {
                    mDeviceExtensionNames.push_back(extension->getExtensionName());
                }
            }
        }

        #pragma endregion

        #pragma region "Queues"

        std::set<uint32_t> uniqueQueueFamilies;

        const auto queueGraphics = findQueue(mPhysicalDevice, vk::QueueFlagBits::eGraphics);
        if (queueGraphics.has_value())
        {
            uniqueQueueFamilies.insert(queueGraphics->familyIndex);
        }

        const auto queueCompute = findQueue(mPhysicalDevice, vk::QueueFlagBits::eCompute, vk::QueueFlagBits::eGraphics);
        if (queueCompute.has_value())
        {
            uniqueQueueFamilies.insert(queueCompute->familyIndex);
        }

        constexpr float queuePriority = 1.0f;

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        for (const uint32_t familyIndex : uniqueQueueFamilies)
        {
            const auto queueCreateInfo = vk::DeviceQueueCreateInfo()
                .setQueueFamilyIndex(familyIndex)
                .setQueueCount(1)
                .setPQueuePriorities(&queuePriority);
            queueCreateInfos.push_back(queueCreateInfo);
        }

        #pragma endregion

        const auto deviceFeatures = getBaseDeviceFeatures();

        auto createInfo = vk::DeviceCreateInfo()
            .setEnabledExtensionCount(mDeviceExtensionNames.size())
            .setPpEnabledExtensionNames(mDeviceExtensionNames.data())
            .setQueueCreateInfoCount(queueCreateInfos.size())
            .setPQueueCreateInfos(queueCreateInfos.data())
            .setPEnabledFeatures(&deviceFeatures);

        for (const auto& extension : mDeviceExtensions)
        {
            extension->preCreateDevice(createInfo);
        }

        nbl_VK_TRY(mDevice = mPhysicalDevice.createDevice(createInfo);)

        mGraphicsQueue = createQueue({
            .queueFamilyIndex = queueGraphics->familyIndex,
            .queueIndex = 0,
            .name = "Graphics Queue",
        });

        mAsyncComputeQueue = createQueue({
            .queueFamilyIndex = queueCompute->familyIndex,
            .queueIndex = 0,
            .name = "Compute Queue",
        });
    }

    void Device::createAllocator()
    {
        const VmaAllocatorCreateInfo createInfo = {
            .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
            .physicalDevice = mPhysicalDevice,
            .device = mDevice,
            .instance = mInstance,
            .vulkanApiVersion = VK_API_VERSION_1_3,
        };

        nbl_VK_C_RESULT(vmaCreateAllocator(&createInfo, &mAllocator));
    }

    std::unique_ptr<Queue> Device::createQueue(const QueueCreateInfo& createInfo) const
    {
        vk::Queue queue;
        nbl_VK_TRY(queue = mDevice.getQueue(createInfo.queueFamilyIndex, createInfo.queueIndex);)

        auto vkQueue = std::make_unique<Queue>();
        vkQueue->queue = queue,
        vkQueue->familyIndex = createInfo.queueFamilyIndex,
        vkQueue->queueIndex = createInfo.queueIndex,
        vkQueue->name = createInfo.name,
        vkQueue->device = mDevice,
        vkQueue->available = true,

        nameObject<vk::Queue>({
            .debugName = vkQueue->name,
            .handle    = vkQueue->queue,
        });

        return vkQueue;
    }
}
