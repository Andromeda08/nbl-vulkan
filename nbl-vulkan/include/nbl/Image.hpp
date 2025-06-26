#pragma once

#include <string>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include "Util.hpp"

namespace nbl
{

    class Device;
    class Image;
    class Swapchain;

    // Immutable properties of a Vulkan Image
    struct ImageProperties
    {
        vk::Format                 format            = vk::Format::eR32G32B32A32Sfloat;
        vk::Extent2D               extent            = { 1920, 1080 };
        vk::SampleCountFlagBits    sampleCount       = vk::SampleCountFlagBits::e1;
        vk::ImageSubresourceRange  subresourceRange  = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };
        vk::ImageSubresourceLayers subresourceLayers = { vk::ImageAspectFlagBits::eColor, 0, 0, 1 };
    };

    // Mutable state of a Vulkan Image
    struct ImageState
    {
        vk::AccessFlags2        accessFlags = vk::AccessFlagBits2::eNone;
        vk::ImageLayout         layout      = vk::ImageLayout::eUndefined;
        vk::PipelineStageFlags2 stageFlags  = vk::PipelineStageFlagBits2::eNone;
    };

    struct ImageCreateInfo
    {
        vk::Extent2D            extent       = {1920, 1080};
        vk::Format              format       = vk::Format::eR32G32B32A32Sfloat;
        vk::SampleCountFlagBits sampleCount  = vk::SampleCountFlagBits::e1;
        vk::ImageTiling         tiling       = vk::ImageTiling::eOptimal;
        vk::ImageUsageFlags     usageFlags   = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
        std::string             debugName    = "Unknown Image";
        bool                    imageSampler = false;
        Device*                 pDevice      = nullptr;
    };

    struct SwapchainImageWrapperInfo
    {
        vk::Image        image;
        vk::ImageView    imageView;
        uint32_t         imageIndex;
        Device*          pDevice;
        Swapchain*       pSwapchain;
    };

    struct BlitImageInfo
    {
        vk::CommandBuffer commandBuffer;
        Image*            dstImage;
    };

    class Image
    {
    public:
        nbl_DISABLE_COPY(Image);
        nbl_CI_CTOR(Image, ImageCreateInfo);

        explicit Image(const SwapchainImageWrapperInfo& createInfo);

        static std::unique_ptr<Image> createSwapchainImageWrapper(const SwapchainImageWrapperInfo& createInfo);

        ~Image();

        void updateState(const ImageState& state)
        {
            mState = state;
        }

        /**
         * @note This operation will transition the srcImage to TransferSrcOptimal and dstImage to TransferDstOptimal if necessary.
         */
        void blitImage(const BlitImageInfo& blitInfo);

        const vk::Image&        getImage()      const { return mImage; }
        const vk::ImageView&    getImageView()  const { return mImageView; }
        const vk::Sampler&      getSampler()    const { return mSampler; }
        const ImageProperties&  getProperties() const { return mProperties; }
        ImageState              getState()      const { return mState; }

        static bool isDepthFormat(vk::Format format);

    private:
        static ImageProperties makeProperties(const ImageCreateInfo& imageInfo);

        vk::Image               mImage;
        vk::ImageView           mImageView;
        vk::Sampler             mSampler;
        ImageState              mState;

        vk::DescriptorImageInfo mDescriptorInfo;

        VmaAllocation           mAllocation {nullptr};
        VmaAllocationInfo       mAllocationInfo {};

        Device*                 mDevice;
        const bool              mSwapchainImage {false};
        const uint32_t          mSwapchainImageIndex {0};
        const ImageProperties   mProperties;
        const std::string       mDebugName;
    };
}