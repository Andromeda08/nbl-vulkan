#pragma once

#include <cstdint>
#include <string>
#include <vulkan/vulkan.hpp>

namespace nbl
{
    struct DebugLabelInfo
    {
        std::array<float, 3> color;
        std::string          label;
    };

    struct QueueCreateInfo
    {
        uint32_t    queueFamilyIndex;
        uint32_t    queueIndex;
        std::string name;
    };

    struct QueueProperties
    {
        vk::QueueFamilyProperties familyProperties;
        uint32_t                  familyIndex;
    };

    struct Queue
    {
        vk::Queue    queue;
        vk::Device   device;
        uint32_t     familyIndex    = 0;
        uint32_t     queueIndex     = 0;
        std::string  name           = "Unknown Queue";
        bool         available      = false;
    };
}