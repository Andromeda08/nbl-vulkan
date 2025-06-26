#pragma once

#include <string>
#include <vector>
#include <ext/nbl-reflect/include/nbl/reflect/ShaderReflection.hpp>

namespace nbl
{
    class Device;

    enum class PipelineType
    {
        Compute,
        Graphics,
        RayTracing,
    };

    struct PipelineCreateInfo
    {
        std::vector<std::string>             shaders;
        PipelineType                         pipelineType;
        Device*                              pDevice;
        std::string                          name;
    };

    // ==============================
    // Pipeline
    // ==============================
    class Pipeline
    {
    public:
        explicit Pipeline(const PipelineCreateInfo& createInfo)
        : mDevice(createInfo.pDevice)
        {
            const auto reflectionData = ShaderReflection::reflectPipelineShaders(createInfo.shaders);

            createPipelineLayout(createInfo, reflectionData);
        }

        void createPipelineLayout(const PipelineCreateInfo& createInfo, const PipelineReflectionData& reflectionData)
        {
            // ==============================
            // Process Descriptor Sets
            // ==============================
            std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
            for (const auto& reflectSet : reflectionData.descriptorSets)
            {
                std::vector<vk::DescriptorSetLayoutBinding> reflectBindings;
                for (const auto& b : reflectSet.bindings)
                {
                    const auto reflectBinding = vk::DescriptorSetLayoutBinding()
                        .setBinding(b.binding)
                        .setDescriptorCount(b.descriptorCount)
                        .setDescriptorType(b.descriptorType)
                        .setStageFlags(b.stageFlags);

                    reflectBindings.push_back(reflectBinding);
                }

                const auto layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
                    .setPBindings(reflectBindings.data())
                    .setBindingCount(reflectBindings.size());

                const auto reflectLayout = mDevice->handle().createDescriptorSetLayout(layoutCreateInfo);
                descriptorSetLayouts.push_back(reflectLayout);
            }

            // ==============================
            // Process Push Constants
            // ==============================
            std::vector<vk::PushConstantRange> pushConstants = reflectionData.pushConstants;

            const auto pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
                .setSetLayoutCount(descriptorSetLayouts.size())
                .setPSetLayouts(descriptorSetLayouts.data())
                .setPushConstantRangeCount(pushConstants.size())
                .setPPushConstantRanges(pushConstants.data());

            // mDevice->handle().createPipelineLayout(pipelineLayoutCreateInfo);
        }

    private:
        vk::PipelineLayout mPipelineLayout;

        Device*            mDevice;
    };

    /* PipelineCreation-ValidUsage-01
     * When specifying a descriptor set layout for set X, the layout must be identical to the one found in a shader.
     * (Identical layout specified multiple times for set X are ignored.)
     */

    /* PipelineCreation-ValidUsage-02
     * When specifying a push constant range the size and offset must be identical to the ones found a shader.
     * (Identical push constant ranges specified multiple times are ignored.)
     */
}