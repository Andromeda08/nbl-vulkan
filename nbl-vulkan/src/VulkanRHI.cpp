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
        VULKAN_HPP_DEFAULT_DISPATCHER.init(mInstance);

        mDevice = Device::createDevice({
            .instance = mInstance,
        });
        VULKAN_HPP_DEFAULT_DISPATCHER.init(mDevice->getHandle());

        mGraphicsQueue = CommandQueue::createCommandQueue({
            .commandListCount           = 2,
            .enableSingleTimeSubmission = true,
            .pDevice                    = mDevice.get(),
            .pQueue                     = mDevice->getGraphicsQueue(),
        });

        mComputeQueue = CommandQueue::createCommandQueue({
            .commandListCount           = 2,
            .enableSingleTimeSubmission = false,
            .pDevice                    = mDevice.get(),
            .pQueue                     = mDevice->getAsyncComputeQueue(),
        });

        mSwapchain = Swapchain::createSwapchain({
            .pWindow = mWindow,
            .pDevice = mDevice.get(),
            .instance = mInstance,
            .imageCount = createInfo.configuration.backBufferCount,
        });

        mImageReady.resize(mBackBufferCount);
        mRenderingFinished.resize(mBackBufferCount);
        mFrameInFlight.resize(mBackBufferCount);

        for (uint32_t i = 0; i < mBackBufferCount; i++)
        {
            auto semaphoreCreateInfo = vk::SemaphoreCreateInfo();
            auto fenceCreateInfo = vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled);

            mImageReady[i]        = mDevice->getHandle().createSemaphore(semaphoreCreateInfo);
            mRenderingFinished[i] = mDevice->getHandle().createSemaphore(semaphoreCreateInfo);
            mFrameInFlight[i]     = mDevice->getHandle().createFence(fenceCreateInfo);
        }
    }

    Frame VulkanRHI::beginFrame() const
    {
        const vk::Fence fence = mFrameInFlight[mCurrentFrame];

        vk::Result result = mDevice->getHandle().waitForFences(1, &fence, true, std::numeric_limits<uint64_t>::max());
        result = mDevice->getHandle().resetFences(1, &fence);

        const auto nextImage = mDevice->getHandle().acquireNextImageKHR(
            mSwapchain->handle(),std::numeric_limits<uint64_t>::max(),
            mImageReady[mCurrentFrame], nullptr).value;

        return {
            .currentFrame       = mCurrentFrame,
            .acquiredImageIndex = nextImage,
        };
    }

    void VulkanRHI::submitFrame(const Frame& frame)
    {
        const auto frameIndex = frame.currentFrame;

        std::vector<vk::CommandBufferSubmitInfo> commandBufferSubmitInfos;
        for (const auto commandBuffer : frame.commandBuffers)
        {
            const auto info = vk::CommandBufferSubmitInfo()
                .setCommandBuffer(commandBuffer);
            commandBufferSubmitInfos.push_back(info);
        }

        const auto waitSemaphoreInfo = vk::SemaphoreSubmitInfo()
            .setSemaphore(mImageReady[frameIndex])
            .setStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput);
        std::vector waitSemaphoreInfos = { waitSemaphoreInfo };

        const auto signalSemaphoreInfo = vk::SemaphoreSubmitInfo()
            .setSemaphore(mRenderingFinished[frameIndex])
            .setStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput);
        std::vector signalSemaphoreInfos = { signalSemaphoreInfo };

        const auto submitInfo = vk::SubmitInfo2()
            .setCommandBufferInfos(commandBufferSubmitInfos)
            .setCommandBufferInfoCount(commandBufferSubmitInfos.size())
            .setWaitSemaphoreInfos(waitSemaphoreInfos)
            .setWaitSemaphoreInfoCount(waitSemaphoreInfos.size())
            .setSignalSemaphoreInfos(signalSemaphoreInfos)
            .setSignalSemaphoreInfoCount(signalSemaphoreInfos.size());

        if (const auto result = mDevice->getGraphicsQueue()->queue.submit2(1, &submitInfo, mFrameInFlight[frameIndex]);
            result != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to submit CommandList");
        }

        mSwapchain->present(mRenderingFinished[frameIndex], frame.acquiredImageIndex);

        mDevice->waitIdle();
        mCurrentFrame = (mCurrentFrame + 1) % mBackBufferCount;
    }

    std::unique_ptr<Buffer> VulkanRHI::createBuffer(const BufferCreateInfo& createInfo) const
    {
        BufferCreateInfo bufferCreateInfo = createInfo;
        bufferCreateInfo.pDevice = mDevice.get();
        return Buffer::createBuffer(bufferCreateInfo);
    }

    std::unique_ptr<Descriptor> VulkanRHI::createDescriptor(const DescriptorCreateInfo& createInfo) const
    {
        DescriptorCreateInfo descriptorCreateInfo = createInfo;
        descriptorCreateInfo.pDevice = mDevice.get();
        return Descriptor::createDescriptor(descriptorCreateInfo);
    }

    std::unique_ptr<Image> VulkanRHI::createImage(const ImageCreateInfo& createInfo) const
    {
        ImageCreateInfo imageCreateInfo = createInfo;
        imageCreateInfo.pDevice = mDevice.get();
        return Image::createImage(imageCreateInfo);
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

        std::vector<const char*> extensions { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
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
