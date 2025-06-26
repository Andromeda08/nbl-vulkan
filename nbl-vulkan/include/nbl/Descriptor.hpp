#pragma once

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "Util.hpp"

namespace nbl
{
    class Device;

    enum class DescriptorType
    {
        CombinedImageSampler,
        StorageBuffer,
        StorageImage,
        UniformBuffer,
        AccelerationStructure,
    };

    struct DescriptorWriteInfo
    {
        uint32_t                              setIndex {0};
        std::vector<vk::WriteDescriptorSet>   writes;
        std::vector<vk::DescriptorBufferInfo> bufferInfos;
        std::vector<vk::DescriptorImageInfo>  imageInfos;
        std::vector<vk::WriteDescriptorSetAccelerationStructureKHR> accelerationStructureInfos;

        DescriptorWriteInfo& setSetIndex(const uint32_t index)
        {
            setIndex = index;
            return *this;
        }

        DescriptorWriteInfo& writeAccelerationStructure(const uint32_t b, const vk::WriteDescriptorSetAccelerationStructureKHR& accelerationStructure, const uint32_t count)
        {
            accelerationStructureInfos.push_back(accelerationStructure);
            const auto write = vk::WriteDescriptorSet()
                .setDstBinding(b)
                .setDescriptorCount(count)
                .setDescriptorType(vk::DescriptorType::eAccelerationStructureKHR)
                .setDstArrayElement(0)
                .setPNext(&accelerationStructure);
            writes.push_back(write);
            return *this;
        }

        DescriptorWriteInfo& writeUniformBuffers(const uint32_t binding, const uint32_t bufferInfoCount, const vk::DescriptorBufferInfo* pBufferInfos)
        {
            const auto write = vk::WriteDescriptorSet()
                .setDstBinding(binding)
                .setDescriptorCount(bufferInfoCount)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setDstArrayElement(0)
                .setPBufferInfo(pBufferInfos);
            writes.push_back(write);
            return *this;
        }

        DescriptorWriteInfo& writeStorageBuffer(const uint32_t b, const vk::DescriptorBufferInfo& bufferInfo, const uint32_t count)
        {
            bufferInfos.push_back(bufferInfo);
            const auto write = vk::WriteDescriptorSet()
                .setDstBinding(b)
                .setDescriptorCount(count)
                .setDescriptorType(vk::DescriptorType::eStorageBuffer)
                .setDstArrayElement(0)
                .setPBufferInfo(&bufferInfos.back());
            writes.push_back(write);
            return *this;
        }

        DescriptorWriteInfo& writeCombinedImageSamplers(const uint32_t binding, const uint32_t imageInfoCount, const vk::DescriptorImageInfo* pImageInfos)
        {
            const auto write = vk::WriteDescriptorSet()
                .setDstBinding(binding)
                .setDescriptorCount(imageInfoCount)
                .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                .setDstArrayElement(0)
                .setPImageInfo(pImageInfos);
            writes.push_back(write);
            return *this;
        }

        DescriptorWriteInfo& writeStorageImages(const uint32_t binding, const uint32_t imageInfoCount, const vk::DescriptorImageInfo* pImageInfos)
        {
            const auto write = vk::WriteDescriptorSet()
                .setDstBinding(binding)
                .setDescriptorCount(imageInfoCount)
                .setDescriptorType(vk::DescriptorType::eStorageImage)
                .setDstArrayElement(0)
                .setPImageInfo(pImageInfos);
            writes.push_back(write);
            return *this;
        }
    };

    struct DescriptorCreateInfo
    {
        std::vector<vk::DescriptorSetLayoutBinding> bindings         = {};
        uint32_t                                    setCount         = 1;
        std::optional<DescriptorWriteInfo>          initialWriteInfo = std::nullopt;
        std::string                                 debugName        = "Unknown Descriptor";
        Device*                                     pDevice          = nullptr;
    };

    class Descriptor
    {
    public:
        nbl_DISABLE_COPY(Descriptor);
        nbl_CI_CTOR(Descriptor, DescriptorCreateInfo);

        ~Descriptor();

        void write(DescriptorWriteInfo writeInfo) const;

        const vk::DescriptorSet&        getSet(size_t i)     const;
        const vk::DescriptorSet&        operator[](size_t i) const;
        const vk::DescriptorSetLayout&  getLayout()          const { return mLayout;   }
        uint32_t                        getSetCount()        const { return mSetCount; }

    private:
        void createPool();
        void createLayout();
        void createSets();

        std::vector<vk::DescriptorSet>              mDescriptorSets;
        std::vector<vk::DescriptorSetLayoutBinding> mBindings;
        vk::DescriptorSetLayout                     mLayout;
        vk::DescriptorPool                          mDescriptorPool;

        const uint32_t                              mSetCount;
        const std::string                           mDebugName;

        Device*                                     mDevice;
    };
}
