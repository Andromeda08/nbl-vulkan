#pragma once

#include <cstdint>
#include <memory>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include <nbl/Pipeline.hpp>
#include <nbl/RenderPass.hpp>
#include <nbl/VulkanRHI.hpp>

namespace nbl
{
    class HairModel;

    struct Frame;

    struct PushConstant
    {
        glm::mat4 model;

        glm::vec4 hairDiffuse;
        glm::vec4 hairSpecular;

        int32_t   vertexCount;
        int32_t   strandCount;
        int32_t   renderMode {0};
        int32_t   _pad0 {-1};

        uint64_t  vertexBuffer;
        uint64_t  strandDescBuffer;

        static vk::PushConstantRange getPushConstantRange()
        {
            return vk::PushConstantRange()
                .setSize(sizeof(PushConstant))
                .setOffset(0)
                .setStageFlags(sShaderStages);
        }

        constexpr static vk::ShaderStageFlags sShaderStages =
            vk::ShaderStageFlagBits::eTaskEXT |
            vk::ShaderStageFlagBits::eMeshEXT |
            vk::ShaderStageFlagBits::eFragment;
    };

    class HairPipeline
    {
    public:
        explicit HairPipeline(VulkanRHI* pRHI, Descriptor* pSceneDescriptor);

        void renderHairModel(
            const HairModel* pHairModel,
            const CommandList*     pCommandList,
            const Frame&     frameInfo) const;

    private:
        std::unique_ptr<Image>      mDepthBuffer;
        std::unique_ptr<RenderPass> mRenderPass;
        std::unique_ptr<Pipeline>   mPipeline;

        Descriptor*                 mDescriptor;

        VulkanRHI*                  mRHI;
    };
}
