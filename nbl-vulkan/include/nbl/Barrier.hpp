#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

namespace nbl
{
    class Image;

    struct ImageTransitionInfo
    {
        Image*                      pImage           = nullptr;
        vk::ImageLayout             newLayout        = vk::ImageLayout::eUndefined;

        vk::ImageSubresourceRange   subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };

        vk::AccessFlags2            srcAccessMask    = vk::AccessFlagBits2::eNone;
        vk::AccessFlags2            dstAccessMask    = vk::AccessFlagBits2::eNone;
        vk::PipelineStageFlags2     srcStageMask     = vk::PipelineStageFlagBits2::eNone;
        vk::PipelineStageFlags2     dstStageMask     = vk::PipelineStageFlagBits2::eNone;
    };

    struct ImageLayoutTransitionInfo
    {
        vk::CommandBuffer           commandBuffer;
        ImageTransitionInfo         imageTransitionInfo;
    };

    struct ImageLayoutTransitionsInfo
    {
        vk::CommandBuffer                commandBuffer;
        std::vector<ImageTransitionInfo> imageTransitionInfos;
    };

    class Barrier
    {
    public:
        static void transitionImageLayout(const ImageLayoutTransitionInfo& transitionInfo);

        static void transitionImageLayouts(const ImageLayoutTransitionsInfo& transitionInfo);
    };
}