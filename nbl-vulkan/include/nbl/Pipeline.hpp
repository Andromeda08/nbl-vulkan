#pragma once

#include <functional>
#include <string>
#include <vulkan/vulkan.hpp>
#include "Util.hpp"

namespace nbl
{
    class Device;
    class RenderPass;

    enum class PipelineType
    {
        Graphics,
        Compute,
        RayTracing,
    };

    std::string toString(PipelineType pipelineType) noexcept;

    struct PipelineUtils
    {
        static vk::PipelineInputAssemblyStateCreateInfo makeInputAssemblyState();

        static vk::PipelineRasterizationStateCreateInfo makeRasterizationState();

        static vk::PipelineMultisampleStateCreateInfo makeMultisampleState();

        static vk::PipelineDepthStencilStateCreateInfo makeDepthStencilState();

        static vk::PipelineViewportStateCreateInfo makeViewportState();

        static vk::PipelineDynamicStateCreateInfo makeDynamicState();

        static vk::PipelineColorBlendStateCreateInfo makeColorBlendState();

        static vk::PipelineVertexInputStateCreateInfo makeVertexInputState();

        using Clr = vk::ColorComponentFlagBits;
        static vk::PipelineColorBlendAttachmentState makeColorBlendAttachmentState(
            vk::ColorComponentFlags colorWriteMask      = Clr::eR | Clr::eG | Clr::eB | Clr::eA,
            vk::Bool32              blendEnable         = false,
            vk::BlendFactor         srcColorBlendFactor = vk::BlendFactor::eOne,
            vk::BlendFactor         dstColorBlendFactor = vk::BlendFactor::eZero,
            vk::BlendOp             colorBlendOp        = vk::BlendOp::eAdd,
            vk::BlendFactor         srcAlphaBlendFactor = vk::BlendFactor::eOne,
            vk::BlendFactor         dstAlphaBlendFactor = vk::BlendFactor::eZero,
            vk::BlendOp             alphaBlendOp        = vk::BlendOp::eAdd);
    };

    struct ShaderCreateInfo
    {
        const char*             filePath    = {};
        vk::ShaderStageFlagBits shaderStage = {};
        const char*             entryPoint  = "main";
    };

    struct GraphicsPipelineStateInfo
    {
        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState = PipelineUtils::makeInputAssemblyState();
        vk::PipelineRasterizationStateCreateInfo rasterizationState = PipelineUtils::makeRasterizationState();
        vk::PipelineMultisampleStateCreateInfo   multisampleState   = PipelineUtils::makeMultisampleState();
        vk::PipelineDepthStencilStateCreateInfo  depthStencilState  = PipelineUtils::makeDepthStencilState();
        vk::PipelineViewportStateCreateInfo      viewportState      = PipelineUtils::makeViewportState();
        vk::PipelineDynamicStateCreateInfo       dynamicState       = PipelineUtils::makeDynamicState();
        vk::PipelineColorBlendStateCreateInfo    colorBlendState    = PipelineUtils::makeColorBlendState();
        vk::PipelineVertexInputStateCreateInfo   vertexInputState   = PipelineUtils::makeVertexInputState();

        std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eScissor, vk::DynamicState::eViewport};

        std::vector<vk::VertexInputAttributeDescription>    attributeDescriptions;
        std::vector<vk::VertexInputBindingDescription>      bindingDescriptions;
        std::vector<vk::PipelineColorBlendAttachmentState>  attachmentStates;

        void update()
        {
            colorBlendState.setAttachmentCount(static_cast<uint32_t>(attachmentStates.size()));
            colorBlendState.setPAttachments(attachmentStates.data());

            vertexInputState.setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDescriptions.size()));
            vertexInputState.setPVertexAttributeDescriptions(attributeDescriptions.data());

            vertexInputState.setVertexBindingDescriptionCount(static_cast<uint32_t>(bindingDescriptions.size()));
            vertexInputState.setPVertexBindingDescriptions(bindingDescriptions.data());

            dynamicState.setDynamicStateCount(static_cast<uint32_t>(dynamicStates.size()));
            dynamicState.setPDynamicStates(dynamicStates.data());
        }

        template <class T>
        GraphicsPipelineStateInfo& addAttributeDescriptions(uint32_t base_location = 0, uint32_t binding = 0)
        {
            for (const auto& attribute : T::attribute_descriptions(base_location, binding))
            {
                attributeDescriptions.push_back(attribute);
            }
            return *this;
        }

        template <class T>
        GraphicsPipelineStateInfo& addBindingDescriptions(uint32_t binding = 0)
        {
            bindingDescriptions.push_back(T::binding_description(binding));
            return *this;
        }

        GraphicsPipelineStateInfo& setCullMode(const vk::CullModeFlagBits cullMode)
        {
            rasterizationState.setCullMode(cullMode);
            return *this;
        }

        GraphicsPipelineStateInfo& setWireframeMode(const bool value = true)
        {
            rasterizationState.setPolygonMode(value ? vk::PolygonMode::eFill : vk::PolygonMode::eLine);
            return *this;
        }

        GraphicsPipelineStateInfo& configure(const std::function<void(GraphicsPipelineStateInfo&)>& fn)
        {
            fn(*this);
            update();
            return *this;
        }
    };

    struct RenderingInfo
    {
        std::vector<vk::Format> colorAttachmentFormats = {};
        vk::Format              depthFormat            = vk::Format::eUndefined;
        vk::Format              stencilFormat          = vk::Format::eUndefined;
    };

    /**
     * Two methods to define Attachment formats:
     * [1] Manually via "RenderingInfo" struct
     * [2] Automatically via "RenderPass"
     */
    struct PipelineCreateInfo
    {
        std::vector<vk::PushConstantRange>   pushConstantRanges;
        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
        std::vector<ShaderCreateInfo>        shaderCreateInfos;

        PipelineType                         pipelineType          = PipelineType::Graphics;
        GraphicsPipelineStateInfo            graphicsPipelineState = {};
        RenderingInfo                        renderingInfo         = {};
        RenderPass*                          pRenderPass           = nullptr;
        std::string                          debugName             = "Unknown Pipeline";
        Device*                              pDevice               = nullptr;
    };

    class Pipeline
    {
    public:
        nbl_DISABLE_COPY(Pipeline);
        nbl_CI_CTOR(Pipeline, PipelineCreateInfo);

        ~Pipeline();

        void bind(const vk::CommandBuffer& commandList) const
        {
            commandList.bindPipeline(mBindPoint, mPipeline);
        }

        void bindDescriptorSet(const vk::CommandBuffer& commandBuffer, const vk::DescriptorSet& descriptorSet) const
        {
            commandBuffer.bindDescriptorSets(mBindPoint, mPipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
        }

        void bindDescriptorSets(const vk::CommandBuffer& commandBuffer, const std::vector<vk::DescriptorSet>& descriptorSets) const
        {
            commandBuffer.bindDescriptorSets(mBindPoint, mPipelineLayout, 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);
        }

        template <typename T>
        void pushConstants(const vk::CommandBuffer& commandBuffer, vk::ShaderStageFlags stages, vk::DeviceSize offset, const T* p_data) const
        {
            commandBuffer.pushConstants(mPipelineLayout, stages, offset, sizeof(T), p_data);
        }

        const vk::Pipeline&       handle() const { return mPipeline; }
        const vk::PipelineLayout& layout() const { return mPipelineLayout; }

    private:
        static vk::PipelineBindPoint toBindPoint(PipelineType type);

        static std::vector<char> readShaderFile(const char* filePath);

        vk::Pipeline          mPipeline;
        vk::PipelineLayout    mPipelineLayout;
        vk::PipelineBindPoint mBindPoint;

        RenderingInfo         mRenderingInfo;

        PipelineType          mPipelineType;

        Device*               mDevice;
        std::string           mName;
    };
}