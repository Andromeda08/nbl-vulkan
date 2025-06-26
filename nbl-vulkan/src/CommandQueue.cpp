#include "CommandQueue.hpp"

#include <ranges>
#include <stdexcept>
#include <fmt/format.h>

#include "Device.hpp"
#include "Util.hpp"

namespace nbl
{
    CommandList::CommandList(const CommandListCreateInfo& createInfo)
    : mCommandBuffer(createInfo.commandBuffer)
    {
    }
    
    void CommandList::begin()
    {
        if (mIsRecording)
        {
            fmt::println("CommandList is already in recording state.");
            return;
        }
    
        nbl_VK_RESULT(mCommandBuffer.begin(&mBeginInfo));
        mIsRecording = true;
    }
    
    void CommandList::end()
    {
        if (!mIsRecording)
        {
            fmt::println("CommandList is not in a recording state.");
            return;
        }
    
        mCommandBuffer.end();
        mIsRecording = false;
    }
    
    CommandQueue::CommandQueue(const CommandQueueCreateInfo& createInfo)
    : mQueue(createInfo.pQueue)
    , mDevice(createInfo.pDevice)
    , mSingleTimeSubmission(createInfo.enableSingleTimeSubmission)
    {
        mCommandBuffers.resize(createInfo.commandListCount);
    
        const auto poolCreateInfo = vk::CommandPoolCreateInfo()
            .setQueueFamilyIndex(mQueue->familyIndex)
            .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    
        nbl_VK_RESULT(mDevice->getHandle().createCommandPool(&poolCreateInfo, nullptr, &mPool));
    
        const auto bufferAllocateInfo = vk::CommandBufferAllocateInfo()
            .setCommandBufferCount(mCommandBuffers.size())
            .setCommandPool(mPool)
            .setLevel(vk::CommandBufferLevel::ePrimary);
    
        nbl_VK_RESULT(mDevice->getHandle().allocateCommandBuffers(&bufferAllocateInfo,mCommandBuffers.data()));
    
        for (auto&& [i, commandBuffer] : std::views::enumerate(mCommandBuffers))
        {
            const auto name = fmt::format("{} CommandList {}", mQueue->name, i);
            mDevice->nameObject<vk::CommandBuffer>({
                .debugName = name,
                .handle = commandBuffer,
            });
            mCommandLists.push_back(CommandList::createCommandList({
                .commandBuffer = commandBuffer
            }));
        }
    }
    
    CommandQueue::~CommandQueue()
    {
        mDevice->waitIdle();
        mCommandLists.clear();
        mDevice->getHandle().freeCommandBuffers(mPool, mCommandBuffers.size(), mCommandBuffers.data());
        mDevice->getHandle().destroyCommandPool(mPool);
    }
    
    CommandList* CommandQueue::getCommandList(const size_t id) const
    {
        if (id >= mCommandBuffers.size())
        {
            throw std::out_of_range(std::to_string(id));
        }
        return mCommandLists[id].get();
    }
    
    void CommandQueue::executeSingleTimeCommand(const std::function<void(const vk::CommandBuffer&)>& lambda) const
    {
        if (!mSingleTimeSubmission)
        {
            fmt::println("CommandQueue {} does not have single-time command submission enabled", mQueue->name);
            return;
        }
    
        const auto allocateInfo = vk::CommandBufferAllocateInfo()
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandPool(mPool)
            .setCommandBufferCount(1);
    
        vk::CommandBuffer commandBuffer;
        nbl_VK_RESULT(mDevice->getHandle().allocateCommandBuffers(&allocateInfo, &commandBuffer));
    
        nbl_VK_RESULT(commandBuffer.begin(&mSingleTimeBeginInfo));
        lambda(commandBuffer);
        commandBuffer.end();
    
        const auto submitInfo = vk::SubmitInfo()
            .setCommandBufferCount(1)
            .setPCommandBuffers(&commandBuffer);
        nbl_VK_RESULT(mQueue->queue.submit(1, &submitInfo, nullptr));
    
        mQueue->queue.waitIdle();
        mDevice->getHandle().freeCommandBuffers(mPool, 1, &commandBuffer);
    }
}
