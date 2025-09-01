#include "ui/ImGuiRenderer.hpp"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include "Barrier.hpp"
#include "Nebula.hpp"
#include "WSI/Window.hpp"

namespace nbl
{
    ImGuiRenderer::ImGuiRenderer(const ImGuiRendererCreateInfo& createInfo)
    : mFontFile(createInfo.fontPath)
    , mWindow(createInfo.pWindow)
    , mRHI(gRHI)
    , mDevice(gRHI->getDevice())
    , mSwapchain(gRHI->getSwapchain())
    {
        createRenderPasses();
        createFramebuffers();

        mDebugLabel = vk::DebugUtilsLabelEXT()
            .setColor(std::array{ 0.8235f, 0.0588f, 0.2235f, 1.0f })
            .setPLabelName("ImGui");

        createImGuiDescriptorPool();

        createImGuiContext();
    }

    void ImGuiRenderer::renderImGui(const vk::CommandBuffer& commandBuffer, const Frame& currentFrame, const std::function<void()>& uiDraws) const
    {
        commandBuffer.beginDebugUtilsLabelEXT(&mDebugLabel);

        constexpr auto clearValue = vk::ClearValue().setColor({ 0.f, 0.f, 0.f, 0.f });
        const auto beginInfo = vk::RenderPassBeginInfo()
            .setRenderArea(mSwapchain->getArea())
            .setRenderPass(mRenderPass)
            .setClearValueCount(1)
            .setPClearValues(&clearValue)
            .setFramebuffer(mFramebuffers[currentFrame.currentFrame]);

        commandBuffer.beginRenderPass(&beginInfo, vk::SubpassContents::eInline);

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();
        uiDraws();
        ImGui::EndFrame();

        ImGui::Render();
        ImDrawData* drawData = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);

        commandBuffer.endRenderPass();
        commandBuffer.endDebugUtilsLabelEXT();
    }

    void ImGuiRenderer::createRenderPasses()
    {
        auto attachment = vk::AttachmentDescription()
            .setFormat(mSwapchain->getFormat())
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eLoad)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

        auto swapchainColorRef = vk::AttachmentReference()
            .setAttachment(0)
            .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

        auto subpass = vk::SubpassDescription()
            .setColorAttachmentCount(1)
            .setPColorAttachments(&swapchainColorRef)
            .setInputAttachmentCount(0)
            .setPInputAttachments(nullptr)
            .setPResolveAttachments(nullptr)
            .setPDepthStencilAttachment(nullptr)
            .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

        auto subpassDependency = vk::SubpassDependency()
            .setSrcSubpass(VK_SUBPASS_EXTERNAL)
            .setDstSubpass(0)
            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
            .setSrcAccessMask({})
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

        auto renderPassInfo = vk::RenderPassCreateInfo()
            .setAttachmentCount(1)
            .setPAttachments(&attachment)
            .setSubpassCount(1)
            .setPSubpasses(&subpass)
            .setDependencyCount(1)
            .setPDependencies(&subpassDependency);

        if (const vk::Result result = mDevice->getHandle().createRenderPass(&renderPassInfo, nullptr, &mRenderPass);
            result != vk::Result::eSuccess)
        {
            throw std::runtime_error(to_string(result));
        }

        attachment.setLoadOp(vk::AttachmentLoadOp::eClear);

        if (const vk::Result result = mDevice->getHandle().createRenderPass(&renderPassInfo, nullptr, &mRenderPassClearTarget);
            result != vk::Result::eSuccess)
        {
            throw std::runtime_error(to_string(result));
        }
    }

    void ImGuiRenderer::createFramebuffers()
    {
        auto framebufferInfo = vk::FramebufferCreateInfo()
            .setAttachmentCount(1)
            .setHeight(mSwapchain->getExtent().height)
            .setLayers(1)
            .setRenderPass(mRenderPass)
            .setWidth(mSwapchain->getExtent().width);

        mFramebuffers.resize(mSwapchain->getImageCount());
        for (uint32_t i = 0; i < mFramebuffers.size(); i++)
        {
            auto imageView = mSwapchain->getImageView(i);
            framebufferInfo.setPAttachments(&imageView);

            if (const vk::Result result = mDevice->getHandle().createFramebuffer(&framebufferInfo, nullptr, &mFramebuffers[i]);
                result != vk::Result::eSuccess)
            {
                throw std::runtime_error(to_string(result));
            }

            mDevice->nameObject<vk::Framebuffer>({
                .debugName = "ImGui Framebuffer",
                .handle = mFramebuffers[i],
            });
        }
    }

    void ImGuiRenderer::createImGuiDescriptorPool()
    {
        #pragma region PoolSize Array
        const vk::DescriptorPoolSize poolSizes[] {
            { vk::DescriptorType::eSampler, 1000 },
            { vk::DescriptorType::eCombinedImageSampler, 1000 },
            { vk::DescriptorType::eSampledImage, 1000 },
            { vk::DescriptorType::eStorageImage, 1000 },
            { vk::DescriptorType::eUniformTexelBuffer, 1000 },
            { vk::DescriptorType::eStorageTexelBuffer, 1000 },
            { vk::DescriptorType::eUniformBuffer, 1000 },
            { vk::DescriptorType::eStorageBuffer, 1000 },
            { vk::DescriptorType::eUniformBufferDynamic, 1000 },
            { vk::DescriptorType::eStorageBufferDynamic, 1000 },
            { vk::DescriptorType::eInputAttachment, 1000 }
        };
        #pragma endregion

        const auto poolInfo = vk::DescriptorPoolCreateInfo()
                .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
                .setMaxSets(1000 * IM_ARRAYSIZE(poolSizes))
                .setPPoolSizes(poolSizes)
                .setPoolSizeCount(IM_ARRAYSIZE(poolSizes));

        if (const vk::Result result = mDevice->getHandle().createDescriptorPool(&poolInfo, nullptr, &mDescriptorPool);
            result != vk::Result::eSuccess)
        {
            throw std::runtime_error("[Error] Failed to create ImGui DescriptorPool");
        }
    }

    void ImGuiRenderer::createImGuiContext() const
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->AddFontFromFileTTF(mFontFile.c_str(), 16.0f);

        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();

        ImGui_ImplGlfw_InitForVulkan(mWindow->getHandle(), true);
        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = gRHI->getInstance();
        initInfo.PhysicalDevice = mDevice->getPhysicalDevice();
        initInfo.Device = mDevice->getHandle();
        initInfo.QueueFamily = mRHI->getGraphicsQueue()->getQueue().familyIndex;
        initInfo.Queue = mRHI->getGraphicsQueue()->getQueue().queue;
        initInfo.PipelineCache = mPipelineCache;
        initInfo.DescriptorPool = mDescriptorPool;
        initInfo.Subpass = 0;
        initInfo.ImageCount = mSwapchain->getImageCount();
        initInfo.MinImageCount = mSwapchain->getImageCount();
        initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        initInfo.Allocator = nullptr;
        initInfo.CheckVkResultFn = nullptr;
        initInfo.RenderPass = mRenderPass;
        ImGui_ImplVulkan_Init(&initInfo);
    }
}