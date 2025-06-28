#pragma once

#include <memory>
#include <string>

#include <cyHairFile.h>

#include <nbl/Buffer.hpp>
#include <nbl/VulkanRHI.hpp>

#include "HairCommon.h"
#include "Util.hpp"
#include "math/Transform.hpp"

namespace vk
{
    class CommandBuffer;
}

namespace nbl
{

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

        friend class HairPipeline;
        friend class HairUIComponent;

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
        Transform                       mTransform          = {};
        glm::vec4                       mDiffuse            = { 0.32549f, 0.23921f, 0.20784f, 1.0f }; // rgb(83, 61, 53)
        glm::vec4                       mSpecular           = { 0.41568f, 0.30588f, 0.21960f, 1.0f }; // rgb(106, 78, 56)

        uint32_t                        mGroupSize          = 0;
        int32_t                         mGroupSizeOverride  = 0;
        bool                            mEnableOverride     = false;
        HairRenderingMode               mRenderingMode      = HairRenderingMode::Normal;

        VulkanRHI*                      mRHI = nullptr;
    };
}
