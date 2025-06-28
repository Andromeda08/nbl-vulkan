#include "Swapchain.hpp"

#include <fmt/format.h>
#include "Device.hpp"
#include "Image.hpp"
#include "IWindow.hpp"

namespace nbl
{
    Swapchain::Swapchain(const SwapchainCreateInfo& createInfo)
    : IAttachmentSource()
    , mImageCount(createInfo.imageCount)
    , mWindow(createInfo.pWindow)
    , mDevice(createInfo.pDevice)
    , mInstance(createInfo.instance)
    {
        auto [width, height] = mWindow->getFramebufferSize();
        mExtent = vk::Extent2D { width, height };

        createSurface();
        checkSwapchainSupport();
        createSwapchain();
        acquireImages();
        makeDynamicState();

        mAspectRatio = static_cast<float>(width) / static_cast<float>(height);

        for (uint32_t i = 0; i < mImageCount; i++)
        {
            mWrappedImages.push_back(Image::createSwapchainImageWrapper({
                .image = mImages[i],
                .imageView = mImageViews[i],
                .imageIndex = i,
                .pDevice = mDevice,
                .pSwapchain = this,
            }));
        }
    }

    Swapchain::~Swapchain()
    {
        for (const auto& imageView : mImageViews)
        {
            mDevice->getHandle().destroyImageView(imageView);
        }
        mImageViews.clear();

        mDevice->getHandle().destroySwapchainKHR(mSwapchain);

        mInstance.destroySurfaceKHR(mSurface);
    }

    void Swapchain::present(const vk::Semaphore waitSemaphore, const uint32_t imageIndex) const
    {
        const auto presentInfo = vk::PresentInfoKHR()
            .setPWaitSemaphores(&waitSemaphore)
            .setWaitSemaphoreCount(1)
            .setPSwapchains(&mSwapchain)
            .setSwapchainCount(1)
            .setImageIndices(imageIndex)
            .setPResults(nullptr);

        if (const auto result = mDevice->getGraphicsQueue()->queue.presentKHR(&presentInfo);
            result != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to present to swapchain");
        }
    }

    void Swapchain::setScissorViewport(const vk::CommandBuffer& commandList) const
    {
        commandList.setScissor(0, 1, &mCachedScissor);
        commandList.setViewport(0, 1, &mCachedViewport);
    }

    Image* Swapchain::getImage(const size_t i) const
    {
        if (i >= mWrappedImages.size())
        {
            throw std::out_of_range(std::to_string(i));
        }
        return mWrappedImages[i].get();
    }

    vk::Image Swapchain::getVkImage(const size_t i) const
    {
        if (i >= mImages.size())
        {
            throw std::out_of_range(std::to_string(i));
        }
        return mImages[i];
    }

    vk::ImageView Swapchain::getImageView(const size_t i) const
    {
        if (i >= mImageViews.size())
        {
            throw std::out_of_range(std::to_string(i));
        }
        return mImageViews[i];
    }

    vk::ImageView Swapchain::getAttachmentSource() const
    {
        return mImageViews[mLastAcquiredIndex];
    }

    void Swapchain::createSurface()
    {
        mWindow->createVulkanSurface(mInstance, &mSurface);
    }

    void Swapchain::checkSwapchainSupport()
    {
        const auto physicalDevice = mDevice->getPhysicalDevice();

        const vk::SurfaceCapabilitiesKHR        surfaceCaps    = physicalDevice.getSurfaceCapabilitiesKHR(mSurface);
        const std::vector<vk::SurfaceFormatKHR> surfaceFormats = physicalDevice.getSurfaceFormatsKHR(mSurface);
        const std::vector<vk::PresentModeKHR>   presentModes   = physicalDevice.getSurfacePresentModesKHR(mSurface);

        mCurrentTransform = surfaceCaps.currentTransform;

        // Capability Checks
        if (surfaceCaps.minImageCount > mImageCount || surfaceCaps.maxImageCount < mImageCount)
        {
            throw std::runtime_error(fmt::format("Swapchain image count {} out of supported range", mImageCount));
        }

        if (surfaceCaps.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            mExtent = vk::Extent2D()
                .setWidth(surfaceCaps.currentExtent.width)
                .setHeight(surfaceCaps.currentExtent.height);
        }

        const vk::Extent2D min = surfaceCaps.minImageExtent;
        const vk::Extent2D max = surfaceCaps.maxImageExtent;

        auto [width, height] = mWindow->getFramebufferSize();
        mExtent.width = std::clamp(width, min.width, max.width);
        mExtent.height = std::clamp(height, min.height, max.height);

        mArea = vk::Rect2D()
            .setExtent(mExtent)
            .setOffset({ 0, 0 });

        // Surface Format & ColorSpace
        if (surfaceFormats.empty())
        {
            throw std::runtime_error("No surface formats found");
        }
        bool formatOkay = false;
        for (const auto& format : surfaceFormats)
        {
            if (format.format == mFormat && format.colorSpace == mColorSpace)
            {
                formatOkay = true;
            }
        }
        if (!formatOkay)
        {
            throw std::runtime_error("Surface Format error");
        }

        // Present Mode
        if (presentModes.empty())
        {
            throw std::runtime_error("No present modes found");
        }
        if (const auto it = std::ranges::find(presentModes, mPresentMode);
            it == std::end(presentModes))
        {
            throw std::runtime_error("PresentMode error");
        }
    }

    void Swapchain::createSwapchain()
    {
        const auto createInfo = vk::SwapchainCreateInfoKHR()
            .setSurface(mSurface)
            .setMinImageCount(mImageCount)
            .setImageFormat(mFormat)
            .setImageColorSpace(mColorSpace)
            .setImageExtent(mExtent)
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst)
            .setPreTransform(mCurrentTransform)
            .setClipped(true)
            .setOldSwapchain(nullptr)
            .setImageSharingMode(vk::SharingMode::eExclusive)
            .setPresentMode(mPresentMode)
            .setQueueFamilyIndexCount(0)
            .setPQueueFamilyIndices(nullptr);

        nbl_VK_TRY(mSwapchain = mDevice->getHandle().createSwapchainKHR(createInfo);)
    }

    void Swapchain::acquireImages()
    {
        mImages.resize(mImageCount);
        mImages = mDevice->getHandle().getSwapchainImagesKHR(mSwapchain);

        constexpr vk::ComponentMapping componentMapping = {
            vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity
        };

        auto create_info = vk::ImageViewCreateInfo()
            .setComponents(componentMapping)
            .setFormat(mFormat)
            .setSubresourceRange({ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 })
            .setViewType(vk::ImageViewType::e2D);

        mImageViews.resize(mImageCount);
        for (uint32_t i = 0; i < mImageCount; i++)
        {
            create_info.setImage(mImages[i]);
            if (const vk::Result result = mDevice->getHandle().createImageView(&create_info, nullptr, &mImageViews[i]);
                result != vk::Result::eSuccess)
            {
                throw std::runtime_error(fmt::format("Failed to create vk::ImageView #{} for Swapchain", i));
            }

            mDevice->nameObject<vk::Image>({
                .debugName = fmt::format("Swapchain #{}", i),
                .handle    = mImages[i],
            });
            mDevice->nameObject<vk::ImageView>({
                .debugName = fmt::format("Swapchain ImageView #{}", i),
                .handle    = mImageViews[i],
            });
        }
    }

    void Swapchain::makeDynamicState()
    {
        mCachedScissor = vk::Rect2D {{ 0, 0 }, mExtent};

        /**
         * Create a viewport object based on the current state of the Swapchain.
         * The viewport is flipped along the Y axis for GLM compatibility.
         * This requires Maintenance1, which is core Vulkan since API version 1.1.
         * https://www.saschawillems.de/blog/2019/03/29/flipping-the-vulkan-viewport/
         */
        mCachedViewport = vk::Viewport()
            .setX(0.0f)
            .setWidth(static_cast<float>(mExtent.width))
            .setY(static_cast<float>(mExtent.height))
            .setHeight(-1.0f * static_cast<float>(mExtent.height))
            .setMaxDepth(1.0f)
            .setMinDepth(0.0f);
    }
}
