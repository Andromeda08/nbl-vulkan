#include "hair/HairPipeline.hpp"

#include "Barrier.hpp"
#include "Pipeline.hpp"
#include "RenderPass.hpp"
#include "VulkanRHI.hpp"
#include "hair/HairModel.hpp"

namespace nbl
{
    HairPipeline::HairPipeline(VulkanRHI* pRHI, Descriptor* pSceneDescriptor)
    : mDescriptor(pSceneDescriptor)
    , mRHI(pRHI)
    {
        mDepthBuffer = mRHI->createImage({
            .extent         = mRHI->getSwapchain()->getExtent(),
            .format         = vk::Format::eD32Sfloat,
            .sampleCount    = vk::SampleCountFlagBits::e1,
            .tiling         = vk::ImageTiling::eOptimal,
            .usageFlags     = vk::ImageUsageFlagBits::eDepthStencilAttachment,
            .debugName      = "Hair DepthBuffer",
            .imageSampler   = false,
        });

        mRHI->getGraphicsQueue()->executeSingleTimeCommand([&](const vk::CommandBuffer& commandBuffer) -> void {
            Barrier::transitionImageLayout({
                .commandBuffer       = commandBuffer,
                .imageTransitionInfo = {
                    .pImage    = mDepthBuffer.get(),
                    .newLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
                },
            });
        });

        Attachment swapchainAttachment = {
            .pSource        = mRHI->getSwapchain(),
            .clearValue     = vk::ClearValue().setColor({ 0.0f, 0.0f, 0.0f, 1.0f }),
            .imageLayout    = vk::ImageLayout::eColorAttachmentOptimal,
            .loadOp         = vk::AttachmentLoadOp::eClear,
            .storeOp        = vk::AttachmentStoreOp::eStore,
        };

        const Attachment depthAttachment = {
            .pSource        = mDepthBuffer.get(),
            .clearValue     = vk::ClearValue().setDepthStencil({ 1.0f, 0 }),
            .imageLayout    = vk::ImageLayout::eDepthStencilAttachmentOptimal,
            .loadOp         = vk::AttachmentLoadOp::eClear,
            .storeOp        = vk::AttachmentStoreOp::eStore,
        };

        mRenderPass = RenderPass::createRenderPass({
            .renderArea         = mRHI->getSwapchain()->getArea(),
            .colorAttachments   = { swapchainAttachment },
            .depthAttachment    = depthAttachment
        });

        mPipeline = Pipeline::createPipeline({
            .pushConstantRanges     = { PushConstant::getPushConstantRange() },
            .descriptorSetLayouts   = { mDescriptor->getLayout() },
            .shaderCreateInfos      = {
                { "nblHair.task.spv", vk::ShaderStageFlagBits::eTaskEXT  },
                { "nblHair.mesh.spv", vk::ShaderStageFlagBits::eMeshEXT  },
                { "nblHair.frag.spv", vk::ShaderStageFlagBits::eFragment },
            },
            .pipelineType           = PipelineType::Graphics,
            .graphicsPipelineState  = GraphicsPipelineStateInfo({
                .attachmentStates = { PipelineUtils::makeColorBlendAttachmentState() }
            })
            .setCullMode(vk::CullModeFlagBits::eNone)
            .configure([&](GraphicsPipelineStateInfo& info){
                // info.depthStencilState.setDepthTestEnable(false);
            }),
            .pRenderPass            = mRenderPass.get(),
            .debugName              = "Hair",
            .pDevice                = mRHI->getDevice(),
        });
    }

    void HairPipeline::renderHairModel(const HairModel* pHairModel, const CommandList* pCommandList, const Frame& frameInfo) const
    {
        constexpr auto marker = vk::DebugUtilsLabelEXT()
            .setColor(std::array{ 0.45f, 0.15f, 0.95f, 1.0f })
            .setPLabelName("Hair Rendering");

        pCommandList->handle().beginDebugUtilsLabelEXT(marker);

        Barrier::transitionImageLayout({
            .commandBuffer = pCommandList->handle(),
            .imageTransitionInfo = {
                .pImage    = mRHI->getSwapchain()->getImage(frameInfo.acquiredImageIndex),
                .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
            },
        });

        mRenderPass->execute(pCommandList->handle(), [&](const vk::CommandBuffer& commandBuffer) -> void {
            mPipeline->bind(commandBuffer);
            mPipeline->bindDescriptorSet(commandBuffer, mDescriptor->getSet(frameInfo.currentFrame));

            const auto [addrVertex, addrStrandDesc] = pHairModel->getBufferAddresses();
            const PushConstant pushConstant = {
                .model            = pHairModel->mTransform.model(),
                .hairDiffuse      = pHairModel->mDiffuse,
                .hairSpecular     = pHairModel->mSpecular,
                .vertexCount      = pHairModel->getVertexCount(),
                .strandCount      = pHairModel->getStrandCount(),
                .renderMode       = static_cast<int32_t>(pHairModel->mRenderingMode),
                ._pad0            = -1,
                .vertexBuffer     = addrVertex,
                .strandDescBuffer = addrStrandDesc,
            };
            mPipeline->pushConstants<PushConstant>(commandBuffer, PushConstant::sShaderStages, 0, &pushConstant);

            pHairModel->render(commandBuffer);
        });

        pCommandList->handle().endDebugUtilsLabelEXT();
    }
}
