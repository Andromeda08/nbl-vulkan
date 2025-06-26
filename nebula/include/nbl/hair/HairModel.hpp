#pragma once

#include <memory>
#include <string>
#include <cyHairFile.h>
#include "HairCommon.h"
#include "Util.hpp"

namespace vk
{
    class CommandBuffer;
}

namespace nbl
{
    class Buffer;
    class VulkanRHI;

    struct HairModelCreateInfo
    {
        std::string filePath = {};
        VulkanRHI*  pRHI     = nullptr;
    };

    class HairModel
    {
        using Vertex_t = HairVertex;
    public:
        nbl_DISABLE_COPY(HairModel);
        nbl_CI_CTOR(HairModel, HairModelCreateInfo);

        ~HairModel() = default;

        void render(const vk::CommandBuffer& commandBuffer) const;

        const HairBufferAddresses& getBufferAddresses() const { return mBufferAddresses; }

        int32_t getVertexCount() const { return static_cast<int32_t>(mVertices.size()); }

        int32_t getStrandCount() const { return static_cast<int32_t>(mStrands.size()); }

        Buffer* getVertexBuffer() const { return mVertexBuffer.get(); }

        Buffer* getStrandDescriptionsBuffer() const { return mStrandDescriptionsBuffer.get(); }


    private:
        void loadFile();

        void processVertices();

        void processStrands();

        void createBuffers();

        // ================================
        // Hair Meta- and Geometry Data
        // ================================
        std::string                     mName;
        cyHairFile                      mHairFile;

        std::vector<Vertex_t>           mVertices;
        std::vector<int32_t>            mStrandVertexCounts;
        std::vector<Strand>             mStrands;
        std::vector<Strandlet>          mStrandlets;
        std::vector<StrandDescription>  mStrandDescriptions;

        // ================================
        // GPU Hair Data
        // ================================
        std::unique_ptr<Buffer>         mVertexBuffer;
        std::unique_ptr<Buffer>         mStrandDescriptionsBuffer;
        HairBufferAddresses             mBufferAddresses;

        // ================================
        // Rendering Options
        // ================================
        uint32_t                        mGroupSize          = 0;
        uint32_t                        mGroupSizeOverride  = 0;
        HairRenderingMode               mRenderingMode      = HairRenderingMode::Normal;

        VulkanRHI*                      mRHI = nullptr;
    };
}
