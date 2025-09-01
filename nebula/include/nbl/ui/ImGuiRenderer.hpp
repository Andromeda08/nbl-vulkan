#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vulkan/vulkan.hpp>

#include "Frame.hpp"
#include "Util.hpp"
#include "VulkanRHI.hpp"
#include "WSI/Window.hpp"

namespace nbl
{
    struct ImGuiRendererCreateInfo
    {
        std::string  fontPath;
        wsi::Window* pWindow;
    };

    class ImGuiRenderer
    {
    public:
        nbl_DISABLE_COPY(ImGuiRenderer);
        nbl_CI_CTOR(ImGuiRenderer, ImGuiRendererCreateInfo);

        ~ImGuiRenderer() = default;

        void renderImGui(const vk::CommandBuffer& commandBuffer, const Frame& currentFrame, const std::function<void()>& uiDraws) const;

    private:
        void createRenderPasses();

        void createFramebuffers();

        void createImGuiDescriptorPool();

        void createImGuiContext() const;

        vk::DescriptorPool                  mDescriptorPool;
        vk::PipelineCache                   mPipelineCache;

        std::vector<vk::Framebuffer>        mFramebuffers;
        uint32_t                            mFramebufferIndex {0};

        vk::RenderPass                      mRenderPass;
        vk::RenderPass                      mRenderPassClearTarget;

        vk::DebugUtilsLabelEXT              mDebugLabel;

        std::string                         mFontFile;

        wsi::Window*                        mWindow;
        VulkanRHI*                          mRHI;
        Device*                             mDevice;
        Swapchain*                          mSwapchain;
    };

}
