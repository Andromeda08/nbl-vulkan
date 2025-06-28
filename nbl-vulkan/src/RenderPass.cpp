#include "RenderPass.hpp"

#include <ranges>

namespace nbl
{
    RenderPass::RenderPass(const RenderPassCreateInfo& createInfo)
    : mColorAttachments(createInfo.colorAttachments)
    , mDepthAttachment(createInfo.depthAttachment)
    , mStencilAttachment(createInfo.stencilAttachment)
    , mRenderArea(createInfo.renderArea)
    , mLabelInfo(createInfo.labelInfo)
    {
        for (auto&& colorAttachment : createInfo.colorAttachments)
        {
            if (colorAttachment.pSource)
            {
                mColorAttachmentInfos.push_back(vk::RenderingAttachmentInfo()
                    .setClearValue(colorAttachment.clearValue)
                    .setImageLayout(colorAttachment.imageLayout)
                    .setImageView(colorAttachment.pSource->getAttachmentSource())
                    .setLoadOp(colorAttachment.loadOp)
                    .setStoreOp(colorAttachment.storeOp)
                );
            }
        }

        if (auto* attachment = createInfo.depthAttachment.pSource;
            attachment != nullptr)
        {
            mDepthAttachmentInfo = vk::RenderingAttachmentInfo()
                .setClearValue(createInfo.depthAttachment.clearValue)
                .setImageLayout(createInfo.depthAttachment.imageLayout)
                .setImageView(attachment->getAttachmentSource())
                .setLoadOp(createInfo.depthAttachment.loadOp)
                .setStoreOp(createInfo.depthAttachment.storeOp);
        }

        if (auto* attachment = createInfo.stencilAttachment.pSource;
            attachment != nullptr)
        {
            mStencilAttachmentInfo = vk::RenderingAttachmentInfo()
                .setClearValue(createInfo.stencilAttachment.clearValue)
                .setImageLayout(createInfo.stencilAttachment.imageLayout)
                .setImageView(attachment->getAttachmentSource())
                .setLoadOp(createInfo.stencilAttachment.loadOp)
                .setStoreOp(createInfo.stencilAttachment.storeOp);
        }

        mRenderingInfo = vk::RenderingInfo()
            .setRenderArea(mRenderArea)
            .setLayerCount(1)
            .setPColorAttachments(mColorAttachmentInfos.data())
            .setColorAttachmentCount(mColorAttachments.size())
            .setPDepthAttachment(&mDepthAttachmentInfo)
            .setPStencilAttachment(&mStencilAttachmentInfo);
    }

    void RenderPass::execute(const vk::CommandBuffer& commandBuffer, const std::function<void(const vk::CommandBuffer&)>& lambda)
    {
        if (mDepthAttachment.pSource)
        {
            mDepthAttachmentInfo.setImageView(mDepthAttachment.pSource->getAttachmentSource());
        }
        if (mStencilAttachment.pSource)
        {
            mStencilAttachmentInfo.setImageView(mStencilAttachment.pSource->getAttachmentSource());
        }

        for (auto&& [attachment, info] : std::views::zip(mColorAttachments, mColorAttachmentInfos))
        {
            if (attachment.pSource)
            {
                info.setImageView(attachment.pSource->getAttachmentSource());
            }
        }

        commandBuffer.beginRendering(&mRenderingInfo);
        lambda(commandBuffer);
        commandBuffer.endRendering();
    }
}
