#include "Pipeline.hpp"

#include <fstream>
#include <fmt/format.h>
#include "Device.hpp"
#include "RenderPass.hpp"

namespace nbl
{
    std::string toString(const PipelineType pipelineType) noexcept
    {
        switch (pipelineType)
        {
        case PipelineType::Graphics:    return "Graphics";
        case PipelineType::Compute:     return "Compute";
        case PipelineType::RayTracing:  return "RayTracing";
        default:                        return "Unknown";
        }
    }

    // ================================
    // Pipeline Utils
    // ================================
    #pragma region

    vk::PipelineInputAssemblyStateCreateInfo PipelineUtils::makeInputAssemblyState()
    {
        return vk::PipelineInputAssemblyStateCreateInfo()
               .setTopology(vk::PrimitiveTopology::eTriangleList)
               .setPrimitiveRestartEnable(false)
               .setFlags({})
               .setPNext(nullptr);
    }

    vk::PipelineRasterizationStateCreateInfo PipelineUtils::makeRasterizationState()
    {
        return vk::PipelineRasterizationStateCreateInfo()
               .setPolygonMode(vk::PolygonMode::eFill)
               .setCullMode(vk::CullModeFlagBits::eBack)
               .setFrontFace(vk::FrontFace::eCounterClockwise)
               .setDepthClampEnable(false)
               .setDepthBiasEnable(false)
               .setDepthBiasClamp(0.0f)
               .setDepthBiasSlopeFactor(0.0f)
               .setLineWidth(1.0f)
               .setRasterizerDiscardEnable(false)
               .setPNext(nullptr);
    }

    vk::PipelineMultisampleStateCreateInfo PipelineUtils::makeMultisampleState()
    {
        return vk::PipelineMultisampleStateCreateInfo()
               .setRasterizationSamples(vk::SampleCountFlagBits::e1)
               .setSampleShadingEnable(false)
               .setPSampleMask(nullptr)
               .setAlphaToCoverageEnable(false)
               .setAlphaToOneEnable(false)
               .setPNext(nullptr);
    }

    vk::PipelineDepthStencilStateCreateInfo PipelineUtils::makeDepthStencilState()
    {
        return vk::PipelineDepthStencilStateCreateInfo()
               .setDepthTestEnable(true)
               .setDepthWriteEnable(true)
               .setDepthCompareOp(vk::CompareOp::eLess)
               .setDepthBoundsTestEnable(false)
               .setStencilTestEnable(false);
    }

    vk::PipelineViewportStateCreateInfo PipelineUtils::makeViewportState()
    {
        return vk::PipelineViewportStateCreateInfo()
               .setViewportCount(1)
               .setPViewports(nullptr)
               .setScissorCount(1)
               .setPScissors(nullptr)
               .setPNext(nullptr);
    }

    vk::PipelineDynamicStateCreateInfo PipelineUtils::makeDynamicState()
    {
        return vk::PipelineDynamicStateCreateInfo()
               .setDynamicStateCount(0)
               .setPDynamicStates(nullptr)
               .setPNext(nullptr);
    }

    vk::PipelineColorBlendStateCreateInfo PipelineUtils::makeColorBlendState()
    {
        return vk::PipelineColorBlendStateCreateInfo()
               .setLogicOp(vk::LogicOp::eClear)
               .setLogicOpEnable(false)
               .setAttachmentCount(0)
               .setPAttachments(nullptr)
               .setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f })
               .setPNext(nullptr);
    }

    vk::PipelineVertexInputStateCreateInfo PipelineUtils::makeVertexInputState()
    {
        return vk::PipelineVertexInputStateCreateInfo()
               .setVertexAttributeDescriptionCount(0)
               .setPVertexAttributeDescriptions(nullptr)
               .setVertexBindingDescriptionCount(0)
               .setPVertexBindingDescriptions(nullptr)
               .setPNext(nullptr);
    }

    vk::PipelineColorBlendAttachmentState PipelineUtils::makeColorBlendAttachmentState(
        const vk::ColorComponentFlags colorWriteMask,
        const vk::Bool32              blendEnable,
        const vk::BlendFactor         srcColorBlendFactor,
        const vk::BlendFactor         dstColorBlendFactor,
        const vk::BlendOp             colorBlendOp,
        const vk::BlendFactor         srcAlphaBlendFactor,
        const vk::BlendFactor         dstAlphaBlendFactor,
        const vk::BlendOp             alphaBlendOp)
    {
        return vk::PipelineColorBlendAttachmentState()
               .setColorWriteMask(colorWriteMask)
               .setBlendEnable(blendEnable)
               .setSrcColorBlendFactor(srcColorBlendFactor)
               .setDstColorBlendFactor(dstColorBlendFactor)
               .setColorBlendOp(colorBlendOp)
               .setSrcAlphaBlendFactor(srcAlphaBlendFactor)
               .setDstAlphaBlendFactor(dstAlphaBlendFactor)
               .setAlphaBlendOp(alphaBlendOp);
    }

    #pragma endregion


    // ================================
    // Pipeline
    // ================================
    #pragma region

    Pipeline::Pipeline(const PipelineCreateInfo& createInfo)
    : mBindPoint(toBindPoint(createInfo.pipelineType))
    , mPipelineType(createInfo.pipelineType)
    , mDevice(createInfo.pDevice)
    , mName(createInfo.debugName)
    {
        PipelineCreateInfo pipelineInfo = createInfo;

        if (createInfo.pipelineType == PipelineType::RayTracing)
        {
            throw std::invalid_argument("Ray Tracing Pipelines are not supported yet.");
        }

        if (createInfo.shaderCreateInfos.empty())
        {
            throw std::runtime_error("Can not create Pipeline with no shaders specified");
        }

        if (createInfo.pipelineType == PipelineType::Graphics)
        {
            auto& graphicsPipelineState = pipelineInfo.graphicsPipelineState;
            graphicsPipelineState.update();
        }

        try
        {
            const auto layoutCreateInfo = vk::PipelineLayoutCreateInfo()
                .setSetLayoutCount(createInfo.descriptorSetLayouts.size())
                .setPSetLayouts(createInfo.descriptorSetLayouts.data())
                .setPushConstantRangeCount(createInfo.pushConstantRanges.size())
                .setPPushConstantRanges(createInfo.pushConstantRanges.data());

            mPipelineLayout = mDevice->getHandle().createPipelineLayout(layoutCreateInfo);
            mDevice->nameObject<vk::PipelineLayout>({
                .debugName = fmt::format("{} Layout", createInfo.debugName),
                .handle    = mPipelineLayout,
            });

        } catch (const vk::SystemError& error) {
            throw error;
        }

        std::vector<vk::ShaderModule> shaders(createInfo.shaderCreateInfos.size());
        std::vector<vk::PipelineShaderStageCreateInfo> shaderStageInfos(createInfo.shaderCreateInfos.size());
        for (size_t index = 0; index < createInfo.shaderCreateInfos.size(); index++)
        {
            const auto& shaderInfo = createInfo.shaderCreateInfos[index];

            const auto shaderSrc = readShaderFile(shaderInfo.filePath);
            auto shaderModuleCreateInfo = vk::ShaderModuleCreateInfo()
                .setCodeSize(sizeof(char) * shaderSrc.size())
                .setPCode(reinterpret_cast<const uint32_t*>(shaderSrc.data()));

            shaders[index] = mDevice->getHandle().createShaderModule(shaderModuleCreateInfo);
            mDevice->nameObject<vk::ShaderModule>({
                .debugName = shaderInfo.filePath,
                .handle    = shaders[index],
            });

            shaderStageInfos[index] = vk::PipelineShaderStageCreateInfo()
                .setStage(shaderInfo.shaderStage)
                .setModule(shaders[index])
                .setPName(shaderInfo.entryPoint);
        }

        if (createInfo.pipelineType == PipelineType::Graphics)
        {
            auto& graphicsPipelineState = pipelineInfo.graphicsPipelineState;
            graphicsPipelineState.update();

            auto graphicsPipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
                .setPInputAssemblyState(&graphicsPipelineState.inputAssemblyState)
                .setPRasterizationState(&graphicsPipelineState.rasterizationState)
                .setPMultisampleState(&graphicsPipelineState.multisampleState)
                .setPDepthStencilState(&graphicsPipelineState.depthStencilState)
                .setPViewportState(&graphicsPipelineState.viewportState)
                .setPDynamicState(&graphicsPipelineState.dynamicState)
                .setPColorBlendState(&graphicsPipelineState.colorBlendState)
                .setPVertexInputState(&graphicsPipelineState.vertexInputState)
                .setStageCount(shaderStageInfos.size())
                .setPStages(shaderStageInfos.data())
                .setLayout(mPipelineLayout)
                .setRenderPass(nullptr)
                .setPNext(nullptr);

            if (createInfo.pRenderPass)
            {
                for (auto&& colorAttachment : createInfo.pRenderPass->mColorAttachments)
                {
                    if (colorAttachment.pSource)
                    {
                        mRenderingInfo.colorAttachmentFormats.push_back(colorAttachment.pSource->getFormat());
                    }
                }

                if (auto* attachment = createInfo.pRenderPass->mDepthAttachment.pSource;
                    attachment != nullptr)
                {
                    mRenderingInfo.depthFormat = attachment->getFormat();
                }

                if (auto* attachment = createInfo.pRenderPass->mStencilAttachment.pSource;
                    attachment != nullptr)
                {
                    mRenderingInfo.stencilFormat = attachment->getFormat();
                }
            }
            else
            {
                mRenderingInfo = createInfo.renderingInfo;
            }

            const auto renderingInfo = vk::PipelineRenderingCreateInfo()
                .setColorAttachmentCount(mRenderingInfo.colorAttachmentFormats.size())
                .setPColorAttachmentFormats(mRenderingInfo.colorAttachmentFormats.data())
                .setDepthAttachmentFormat(mRenderingInfo.depthFormat)
                .setStencilAttachmentFormat(mRenderingInfo.stencilFormat);

            graphicsPipelineCreateInfo
                .setRenderPass(nullptr)
                .setPNext(&renderingInfo);

            nbl_VK_TRY(mPipeline = mDevice->getHandle().createGraphicsPipeline(nullptr, graphicsPipelineCreateInfo).value;)
        }

        if (createInfo.pipelineType == PipelineType::Compute)
        {
            const auto it = std::ranges::find_if(shaderStageInfos, [&](const auto& info){
                return info.stage == vk::ShaderStageFlagBits::eCompute;
            });

            if (it == std::end(shaderStageInfos))
            {
                throw;
            }

            const auto computeCreateInfo = vk::ComputePipelineCreateInfo()
                .setLayout(mPipelineLayout)
                .setStage(*it);

            nbl_VK_RESULT(mDevice->getHandle().createComputePipelines({}, 1, &computeCreateInfo, nullptr, &mPipeline));
        }

        mDevice->nameObject<vk::Pipeline>({
            .debugName = createInfo.debugName,
            .handle    = mPipeline,
        });
    }

    Pipeline::~Pipeline()
    {
        mDevice->getHandle().destroyPipeline(mPipeline);
        mDevice->getHandle().destroyPipelineLayout(mPipelineLayout);
    }

    vk::PipelineBindPoint Pipeline::toBindPoint(const PipelineType type)
    {
        using enum PipelineType;
        switch (type)
        {
            case Compute:    return vk::PipelineBindPoint::eCompute;
            case Graphics:   return vk::PipelineBindPoint::eGraphics;
            case RayTracing: return vk::PipelineBindPoint::eRayTracingKHR;
        }
        throw std::runtime_error("");
    }

    std::vector<char> Pipeline::readShaderFile(const char* filePath)
    {
        std::ifstream file(filePath, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error(fmt::format("Failed to open file: {}!", filePath));
        }

        const size_t fileSize = file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }

    #pragma endregion
}
