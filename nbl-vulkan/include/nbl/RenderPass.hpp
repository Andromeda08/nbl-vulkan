#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "Common.hpp"
#include "IAttachmentSource.hpp"
#include "Util.hpp"

namespace nbl
{
    /**
     * RenderPass Attachment Description
     * Valid attachment sources: Swapchain, Image
     * @note Swapchain always uses the Image View with the last acquired image index).
     */
    struct Attachment
    {
        IAttachmentSource*    pSource     = nullptr;
        vk::ClearValue        clearValue  = vk::ClearValue().setColor({ 0.0f, 0.0f, 0.0f, 1.0f });
        vk::ImageLayout       imageLayout = vk::ImageLayout::eGeneral;
        vk::AttachmentLoadOp  loadOp      = vk::AttachmentLoadOp::eClear;
        vk::AttachmentStoreOp storeOp     = vk::AttachmentStoreOp::eStore;
    };

    struct RenderPassCreateInfo
    {
        vk::Rect2D                    renderArea;
        std::vector<Attachment>       colorAttachments;
        Attachment                    depthAttachment;
        Attachment                    stencilAttachment;
        std::optional<DebugLabelInfo> labelInfo = std::nullopt;
    };

    class RenderPass
    {
    public:
        nbl_DISABLE_COPY(RenderPass);
        nbl_CI_CTOR(RenderPass, RenderPassCreateInfo);

        void execute(
            const vk::CommandBuffer&                             commandBuffer,
            const std::function<void(const vk::CommandBuffer&)>& lambda);

    private:
        friend class Pipeline;

        vk::RenderingInfo                        mRenderingInfo;

        std::vector<Attachment>                  mColorAttachments;
        Attachment                               mDepthAttachment;
        Attachment                               mStencilAttachment;

        vk::Rect2D                               mRenderArea;
        std::vector<vk::RenderingAttachmentInfo> mColorAttachmentInfos;
        vk::RenderingAttachmentInfo              mDepthAttachmentInfo;
        vk::RenderingAttachmentInfo              mStencilAttachmentInfo;

        std::optional<DebugLabelInfo>            mLabelInfo;
    };
}