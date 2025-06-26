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
