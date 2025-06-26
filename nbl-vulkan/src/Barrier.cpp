#include "Barrier.hpp"

#include "Image.hpp"

namespace nbl
{
    void Barrier::transitionImageLayout(const ImageLayoutTransitionInfo& transitionInfo)
    {
        const auto& imageInfo = transitionInfo.imageTransitionInfo;

        const auto barrier = vk::ImageMemoryBarrier2()
            .setOldLayout(imageInfo.pImage->getState().layout)
            .setNewLayout(imageInfo.newLayout)
            .setSrcAccessMask(imageInfo.pImage->getState().accessFlags)
            .setDstAccessMask(imageInfo.dstAccessMask)
            .setSrcStageMask(imageInfo.srcStageMask)
            .setDstStageMask(imageInfo.dstStageMask)
            .setSubresourceRange(imageInfo.pImage->getProperties().subresourceRange)
            .setImage(imageInfo.pImage->getImage());

        const auto dependencyInfo = vk::DependencyInfo()
            .setImageMemoryBarrierCount(1)
            .setPImageMemoryBarriers(&barrier);

        transitionInfo.commandBuffer.pipelineBarrier2(&dependencyInfo);

        imageInfo.pImage->updateState({
            .accessFlags = imageInfo.dstAccessMask,
            .layout      = imageInfo.newLayout,
            .stageFlags  = imageInfo.dstStageMask,
        });
    }

    void Barrier::transitionImageLayouts(const ImageLayoutTransitionsInfo& transitionInfo)
    {
        if (transitionInfo.imageTransitionInfos.empty())
        {
            return;
        }

        std::vector<vk::ImageMemoryBarrier2> barriers;
        for (const auto& imageInfo : transitionInfo.imageTransitionInfos)
        {
            barriers.push_back(vk::ImageMemoryBarrier2()
                .setOldLayout(imageInfo.pImage->getState().layout)
                .setNewLayout(imageInfo.newLayout)
                .setSrcAccessMask(imageInfo.pImage->getState().accessFlags)
                .setDstAccessMask(imageInfo.dstAccessMask)
                .setSrcStageMask(imageInfo.srcStageMask)
                .setDstStageMask(imageInfo.dstStageMask)
                .setSubresourceRange(imageInfo.pImage->getProperties().subresourceRange)
                .setImage(imageInfo.pImage->getImage())
            );

            imageInfo.pImage->updateState({
                .accessFlags = imageInfo.dstAccessMask,
                .layout      = imageInfo.newLayout,
                .stageFlags  = imageInfo.dstStageMask,
            });
        }

        const auto dependencyInfo = vk::DependencyInfo()
            .setImageMemoryBarrierCount(barriers.size())
            .setPImageMemoryBarriers(barriers.data());

        transitionInfo.commandBuffer.pipelineBarrier2(&dependencyInfo);
    }
}
