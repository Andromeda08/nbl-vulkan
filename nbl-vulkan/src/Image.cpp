#include "Image.hpp"

#include <fmt/format.h>
#include "Device.hpp"
#include "Swapchain.hpp"

namespace nbl
{
    Image::Image(const ImageCreateInfo& createInfo)
    : mDevice(createInfo.pDevice)
    , mProperties(makeProperties(createInfo))
    , mDebugName(createInfo.debugName)
    {
        /**
         * Create Image
         */
        auto imageCreateInfo = vk::ImageCreateInfo()
            .setFormat(mProperties.format)
            .setExtent({ mProperties.extent.width, mProperties.extent.height, 1 })
            .setSamples(mProperties.sampleCount)
            .setUsage(createInfo.usageFlags)
            .setTiling(createInfo.tiling)
            .setArrayLayers(1)
            .setMipLevels(1)
            .setImageType(vk::ImageType::e2D)
            .setSharingMode(vk::SharingMode::eExclusive)
            .setInitialLayout(vk::ImageLayout::eUndefined);
    
        VmaAllocationCreateInfo allocationInfo {};
        allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
    
        const auto* pImageInfo = reinterpret_cast<VkImageCreateInfo*>(&imageCreateInfo);
        auto* pImage = reinterpret_cast<VkImage*>(&mImage);
        nbl_VK_C_RESULT(
            vmaCreateImage(mDevice->getAllocator(), pImageInfo, &allocationInfo, pImage, &mAllocation, &mAllocationInfo)
        );
    
        mDevice->nameObject<vk::Image>({
            .debugName = mDebugName,
            .handle = mImage,
        });
    
        /**
         * Create ImageView
         */
        const auto viewCreateInfo = vk::ImageViewCreateInfo()
            .setFormat(mProperties.format)
            .setImage(mImage)
            .setSubresourceRange(mProperties.subresourceRange)
            .setViewType(vk::ImageViewType::e2D);
    
        nbl_VK_TRY(mImageView = mDevice->getHandle().createImageView(viewCreateInfo);)
    
        mDevice->nameObject<vk::ImageView>({
            .debugName = fmt::format("{} View", mDebugName),
            .handle = mImageView,
        });
    
        /**
         * Create Sampler
         */
        if (createInfo.imageSampler)
        {
            constexpr auto samplerCreateInfo = vk::SamplerCreateInfo()
                .setMagFilter(vk::Filter::eLinear)
                .setMinFilter(vk::Filter::eLinear)
                .setAddressModeU(vk::SamplerAddressMode::eRepeat)
                .setAddressModeV(vk::SamplerAddressMode::eRepeat)
                .setAddressModeW(vk::SamplerAddressMode::eRepeat)
                .setAnisotropyEnable(true)
                .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
                .setUnnormalizedCoordinates(false)
                .setCompareEnable(false)
                .setCompareOp(vk::CompareOp::eAlways)
                .setMipmapMode(vk::SamplerMipmapMode::eLinear)
                .setMipLodBias(0.0f)
                .setMinLod(0.0f)
                .setMaxLod(0.0f);
    
            nbl_VK_TRY(mSampler = mDevice->getHandle().createSampler(samplerCreateInfo);)
    
            mDevice->nameObject<vk::Sampler>({
                .debugName = fmt::format("{} Sampler", mDebugName),
                .handle = mSampler,
            });
        }
    }
    
    Image::Image(const SwapchainImageWrapperInfo& createInfo)
    : mImage(createInfo.image)
    , mImageView(createInfo.imageView)
    , mState({})
    , mDevice(createInfo.pDevice)
    , mSwapchainImage(true)
    , mSwapchainImageIndex(createInfo.imageIndex)
    , mProperties({
        .format = createInfo.pSwapchain->getFormat(),
        .extent = createInfo.pSwapchain->getExtent(),
        .sampleCount = vk::SampleCountFlagBits::e1,
    })
    , mDebugName(fmt::format("WrappedSwapchainImage{}", mSwapchainImageIndex))
    {
    }
    
    std::unique_ptr<Image> Image::createSwapchainImageWrapper(const SwapchainImageWrapperInfo& createInfo)
    {
        return std::make_unique<Image>(createInfo);
    }
    
    Image::~Image()
    {
        if (!mSwapchainImage)
        {
            vmaDestroyImage(mDevice->getAllocator(), mImage, mAllocation);
        }
    }
    
    void Image::blitImage(const BlitImageInfo& blitInfo)
    {
        /*
        const auto dstProperties = blitInfo.dstImage->mProperties;
        const auto imageBlit = vk::ImageBlit2()
            .setSrcOffsets({
                vk::Offset3D{0,0,0},
                vk::Offset3D{static_cast<int>(mProperties.extent.width), static_cast<int>(mProperties.extent.height), 1}
            })
            .setSrcSubresource(mProperties.subresourceLayers)
            .setDstOffsets({
                vk::Offset3D{0,0,0},
                vk::Offset3D{static_cast<int>(dstProperties.extent.width), static_cast<int>(dstProperties.extent.height), 1}
            })
            .setDstSubresource(blitInfo.dstImage->mProperties.subresourceLayers);
    
        const auto blitImageInfo = vk::BlitImageInfo2()
            .setSrcImage(mImage)
            .setSrcImageLayout(vk::ImageLayout::eTransferSrcOptimal)
            .setDstImage(blitInfo.dstImage->mImage)
            .setDstImageLayout(vk::ImageLayout::eTransferDstOptimal)
            .setFilter(vk::Filter::eNearest)
            .setRegionCount(1)
            .setPRegions(&imageBlit);
    
        // Only apply necessary barriers
        std::vector<ImageTransitionInfo> transitionInfos;
        if (mState.layout != vk::ImageLayout::eTransferSrcOptimal)
        {
            transitionInfos.push_back({
                .image = this,
                .newLayout = vk::ImageLayout::eTransferSrcOptimal,
            });
        }
        if (blitInfo.dstImage->mState.layout != vk::ImageLayout::eTransferDstOptimal)
        {
            transitionInfos.push_back({
                .image = blitInfo.dstImage,
                .newLayout = vk::ImageLayout::eTransferDstOptimal,
            });
        }
    
        if (!transitionInfos.empty())
        {
            VulkanBarrier::transitionImageLayouts({
                .commandBuffer = blitInfo.commandBuffer,
                .imageTransitionInfos = transitionInfos,
            });
        }
    
        blitInfo.commandBuffer.blitImage2(&blitImageInfo);
        */
    }
    
    ImageProperties Image::makeProperties(const ImageCreateInfo& imageInfo)
    {
        ImageProperties properties = {
            .format = imageInfo.format,
            .extent = imageInfo.extent,
            .sampleCount = imageInfo.sampleCount,
        };
    
        if (isDepthFormat(imageInfo.format))
        {
            properties.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
            properties.subresourceLayers.aspectMask = vk::ImageAspectFlagBits::eDepth;
        }
    
        return properties;
    }
    
    bool Image::isDepthFormat(const vk::Format format)
    {
        static std::set depthFormats = {
            vk::Format::eD16Unorm, vk::Format::eD32Sfloat,
            vk::Format::eD16UnormS8Uint, vk::Format::eD24UnormS8Uint, vk::Format::eD32SfloatS8Uint,
            vk::Format::eX8D24UnormPack32,
        };
        return depthFormats.contains(format);
    }
}
