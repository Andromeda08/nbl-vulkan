#pragma once

#include <cstdint>
#include <initializer_list>
#include <vector>
#include "CommandQueue.hpp"

namespace nbl
{
    struct Frame
    {
        const uint32_t currentFrame;
        const uint32_t acquiredImageIndex;

        std::vector<vk::CommandBuffer> commandBuffers;

        Frame& addCommandLists(const std::initializer_list<vk::CommandBuffer> commandLists)
        {
            commandBuffers.append_range(commandLists);
            return *this;
        }
    };
}
