#include "Descriptor.hpp"

#include <ranges>
#include <stdexcept>
#include <fmt/format.h>
#include "Device.hpp"

namespace nbl
{
    Descriptor::Descriptor(const DescriptorCreateInfo& createInfo)
    : mBindings(createInfo.bindings)
    , mSetCount(createInfo.setCount)
    , mDebugName(createInfo.debugName)
    , mDevice(createInfo.pDevice)
    {
        createPool();
        createLayout();
        createSets();
    
        if (createInfo.initialWriteInfo.has_value())
        {
            const auto& initialWrite = createInfo.initialWriteInfo.value();
            write(initialWrite);
        }
    }
    
    Descriptor::~Descriptor()
    {
        mDevice->getHandle().freeDescriptorSets(mDescriptorPool, mSetCount, mDescriptorSets.data());
        mDevice->getHandle().destroy(mDescriptorPool);
        mDevice->getHandle().destroy(mLayout);
    }
    
    void Descriptor::write(DescriptorWriteInfo writeInfo) const
    {
        if (writeInfo.setIndex >= mSetCount)
        {
            throw std::out_of_range(std::to_string(writeInfo.setIndex));
        }
    
        if (writeInfo.writes.empty())
        {
            fmt::println("Submitted empty DescriptorWriteInfo to Descriptor {}", mDebugName);
            return;
        }
    
        for (auto&& write : writeInfo.writes)
        {
            write.setDstSet(mDescriptorSets[writeInfo.setIndex]);
        }
    
        mDevice->getHandle().updateDescriptorSets(
            writeInfo.writes.size(),
            writeInfo.writes.data(),
            0, nullptr);
    }
    
    const vk::DescriptorSet& Descriptor::getSet(const size_t i) const
    {
        if (i >= mSetCount)
        {
            throw std::out_of_range(std::to_string(i));
        }
        return mDescriptorSets[i];
    }
    
    const vk::DescriptorSet& Descriptor::operator[](const size_t i) const
    {
        return getSet(i);
    }
    
    void Descriptor::createPool()
    {
        std::vector<vk::DescriptorPoolSize> poolSizes;
        for (const auto& binding : mBindings)
        {
            poolSizes.push_back(vk::DescriptorPoolSize().setDescriptorCount(binding.descriptorCount).setType(binding.descriptorType));
        }
    
        const auto poolCreateInfo = vk::DescriptorPoolCreateInfo()
            .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
            .setMaxSets(mSetCount)
            .setPoolSizeCount(poolSizes.size())
            .setPPoolSizes(poolSizes.data());
    
        nbl_VK_TRY(mDescriptorPool = mDevice->getHandle().createDescriptorPool(poolCreateInfo);)
    
        mDevice->nameObject<vk::DescriptorPool>({
            .debugName = fmt::format("{} Pool", mDebugName),
            .handle = mDescriptorPool,
        });
    }
    
    void Descriptor::createLayout()
    {
        const auto createInfo = vk::DescriptorSetLayoutCreateInfo()
            .setBindingCount(mBindings.size())
            .setPBindings(mBindings.data());
    
        nbl_VK_TRY(mLayout = mDevice->getHandle().createDescriptorSetLayout(createInfo);)
    
        mDevice->nameObject<vk::DescriptorSetLayout>({
            .debugName = fmt::format("{} Layout", mDebugName),
            .handle = mLayout,
        });
    }
    
    void Descriptor::createSets()
    {
        mDescriptorSets.resize(mSetCount);
        const std::vector layouts(mSetCount, mLayout);
        const auto allocateInfo = vk::DescriptorSetAllocateInfo()
            .setDescriptorPool(mDescriptorPool)
            .setDescriptorSetCount(mSetCount)
            .setPSetLayouts(layouts.data());
    
        nbl_VK_RESULT(mDevice->getHandle().allocateDescriptorSets(&allocateInfo, mDescriptorSets.data()));
    
        for (const auto& [i, descriptorSet] : std::views::enumerate(mDescriptorSets))
        {
            mDevice->nameObject<vk::DescriptorSet>({
                .debugName = fmt::format("{} Set {}", mDebugName, i),
                .handle    = descriptorSet,
            });
        }
    }
}