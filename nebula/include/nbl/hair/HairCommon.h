#pragma once

#include <cstdint>
#include <span>
#include <stdexcept>
#include <glm/glm.hpp>

namespace nbl
{
    static constexpr int32_t gHAIR_WORKGROUP_SIZE     = 32;
    static constexpr int32_t gHAIR_MAX_STRANDLET_SIZE = gHAIR_WORKGROUP_SIZE;

    enum class HairRenderingMode : int32_t
    {
        Normal          = 0,
        DebugQuads      = 1,
        DebugStrands    = 2,
        DebugStrandlets = 3,
    };

    inline std::string toString(const HairRenderingMode renderingMode)
    {
        using enum HairRenderingMode;

        switch (renderingMode)
        {
        case Normal:            return "Normal";
        case DebugQuads:        return "Debug (Quads)";
        case DebugStrands:      return "Debug (Strands)";
        case DebugStrandlets:   return "Debug (Strandlets)";
        }

        throw std::invalid_argument("Unknown HairRenderingMode");
    }

    // Basic Hair vertex data
    struct HairVertex
    {
        glm::vec4 position;
    };

    // Hair Strand data [CPU Only]
    struct Strand
    {
        int32_t               id         = 0;
        int32_t               pointCount = 0;
        std::span<HairVertex> vertices = {};
    };

    // Hair Strandlet data [CPU Only]
    struct Strandlet
    {
        int32_t               strandId   = 0;   // Refers to a valid Strand
        int32_t               pointCount = 0;
        std::span<HairVertex> vertices   = {};
    };

    // [GPU and CPU]
    struct StrandDescription
    {
        int32_t strandId        = 0;
        int32_t pointCount      = 0;
        int32_t strandletCount  = 0;
        int32_t vertexOffset    = 0;
    };

    // [GPU and CPU]
    struct HairBufferAddresses
    {
        uint64_t vertexBuffer             = 0;
        uint64_t strandDescriptionsBuffer = 0;
    };
}
