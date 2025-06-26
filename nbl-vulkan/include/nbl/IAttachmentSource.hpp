#pragma once

#include <vulkan/vulkan.hpp>

namespace nbl
{
    /**
     * Mechanism for providing ImageView-s for RenderPass attachments.
     * Valid attachment sources: SwapChain and Image.
     */
    class IAttachmentSource
    {
    public:
        virtual vk::ImageView getAttachmentSource() const = 0;
        virtual vk::Format    getFormat()           const = 0;

        virtual ~IAttachmentSource() = default;
    };
}