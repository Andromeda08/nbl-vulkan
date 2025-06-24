#pragma once

namespace nbl
{
    struct QueueProperties
    {
        vk::QueueFamilyProperties familyProperties;
        uint32_t                  familyIndex;
    };
}