#pragma once

#include <cstdint>
#include <functional>
#include <vulkan/vulkan.hpp>
#include "Util.hpp"

namespace nbl
{
    class  Device;
    struct Queue;

    struct CommandListCreateInfo
    {
        vk::CommandBuffer commandBuffer;
    };

    class CommandList
    {
    public:
        nbl_DISABLE_COPY(CommandList);
        nbl_CI_CTOR(CommandList, CommandListCreateInfo);

        void begin();

        void end();

        const vk::CommandBuffer& handle() const { return mCommandBuffer; }

        const vk::CommandBuffer& operator->() const
        {
            return mCommandBuffer;
        }

    private:
        vk::CommandBuffer                mCommandBuffer;

        bool                             mIsRecording   = false;
        const vk::CommandBufferBeginInfo mBeginInfo     = vk::CommandBufferBeginInfo();
    };

    struct CommandQueueCreateInfo
    {
        uint32_t        commandListCount           = 2;
        bool            enableSingleTimeSubmission = true;
        Device*         pDevice                    = nullptr;
        Queue*          pQueue                     = nullptr;
    };

    class CommandQueue
    {
    public:
        nbl_DISABLE_COPY(CommandQueue);
        nbl_CI_CTOR(CommandQueue, CommandQueueCreateInfo);

        ~CommandQueue();

        CommandList* getCommandList(size_t id) const;

        void executeSingleTimeCommand(const std::function<void(const vk::CommandBuffer&)>& lambda) const;

        const Queue& getQueue() const { return *mQueue; }

    private:
        vk::CommandPool                  mPool;
        std::vector<vk::CommandBuffer>   mCommandBuffers;
        Queue*                           mQueue;
        Device*                          mDevice;

        const vk::CommandBufferBeginInfo mSingleTimeBeginInfo  = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        bool                             mSingleTimeSubmission = true;

        std::vector<std::unique_ptr<CommandList>> mCommandLists;

    };

}