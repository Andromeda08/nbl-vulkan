#include "hair/HairModel.hpp"


#include <fmt/format.h>
#include <nbl/Buffer.hpp>
#include <nbl/CommandQueue.hpp>
#include <nbl/VulkanRHI.hpp>

namespace nbl
{
    HairModel::HairModel(const HairModelCreateInfo& createInfo)
    : mName(createInfo.filePath)
    , mRHI(createInfo.pRHI)
    {
        loadFile();
        processVertices();
        processStrands();
        createBuffers();

        mGroupSize = static_cast<uint32_t>(std::floor(getStrandCount() / gHAIR_WORKGROUP_SIZE));

        mTransform.euler = glm::vec3(-90.0f, 0.0f, -45.0f);
    }

    void HairModel::loadFile()
    {
        mHairFile.LoadFromFile(mName.c_str());
    }

    void HairModel::processVertices()
    {
        const float* hairPoints = mHairFile.GetPointsArray();
        for (int32_t i = 0; i < mHairFile.GetHeader().point_count * 3; i += 3) {
            mVertices.emplace_back(glm::vec4(hairPoints[i], hairPoints[i + 1], hairPoints[i + 2], 1.0f));
        }
    }

    void HairModel::processStrands()
    {
        if (mHairFile.GetSegmentsArray() != nullptr)
        {
            const uint16_t* segments_array = mHairFile.GetSegmentsArray();
            for (int32_t i = 0; i < mHairFile.GetHeader().hair_count; i++) {
                mStrandVertexCounts.push_back(segments_array[i]);
            }
        }

        const std::span vertexSpan { mVertices };

        int32_t vertexOffset = 0;
        for (int32_t i = 0; i < mHairFile.GetHeader().hair_count; i++)
        {
            const auto strandVertexCount = static_cast<int32_t>((mStrandVertexCounts.empty())
                ? mHairFile.GetHeader().d_segments + 1
                : mStrandVertexCounts[i] + 1);

            // 1. Strand
            Strand strand {
                .id = i,
                .pointCount = strandVertexCount,
                .vertices = vertexSpan.subspan(vertexOffset, strandVertexCount),
            };
            mStrands.push_back(strand);

            // 2. Process Strandlets
            constexpr int32_t strandletSize = gHAIR_MAX_STRANDLET_SIZE;
            const int32_t strandletCount = std::ceil(static_cast<double>(strandVertexCount) / static_cast<double>(strandletSize));
            std::vector<Strandlet> strandlets;
            for (int32_t j = 0; j < strandletCount; j++)
            {
                const int32_t pointCount = (j != strandletCount - 1) ? 32 : (strandVertexCount - (j * strandletSize));
                Strandlet strandlet {
                    .strandId = i,
                    .pointCount = pointCount,
                    .vertices = strand.vertices.subspan((j * strandletSize), pointCount),
                };
                strandlets.push_back(strandlet);
                mStrandlets.push_back(strandlet);
            }

            // 3. Strand Description
            StrandDescription strand_description {
                .strandId       = i,
                .pointCount     = strand.pointCount,
                .strandletCount = strandletCount,
                .vertexOffset   = vertexOffset,
            };
            mStrandDescriptions.push_back(strand_description);

            vertexOffset += strandVertexCount;
        }
    }

    void HairModel::createBuffers()
    {
        #pragma region "Vertex Buffer"
        const auto vertexSize = sizeof(Vertex_t) * mVertices.size();

        mVertexBuffer = mRHI->createBuffer({
            .size      = vertexSize,
            .type      = BufferType::Storage,
            .debugName = fmt::format("HairModel: {} (Vertices)", mName),
        });

        const auto vertexBufferStaging = mRHI->createBuffer({
            .size      = vertexSize,
            .type      = BufferType::Staging,
        });
        vertexBufferStaging->setData(mVertices.data(), vertexSize);
        #pragma endregion

        #pragma region "StrandDescription Buffer"
        const auto strandDescSize = sizeof(StrandDescription) * mStrandDescriptions.size();

        mStrandDescriptionsBuffer = mRHI->createBuffer({
            .size      = strandDescSize,
            .type      = BufferType::Storage,
            .debugName = fmt::format("HairModel: {} (Strand Descriptions)", mName),
        });

        const auto strandDescStaging = mRHI->createBuffer({
            .size      = strandDescSize,
            .type      = BufferType::Staging,
        });
        strandDescStaging->setData(mStrandDescriptions.data(), strandDescSize);
        #pragma endregion

        mRHI->getGraphicsQueue()->executeSingleTimeCommand([&](const vk::CommandBuffer& commandBuffer) {
            vertexBufferStaging->copy({
                .pDstBuffer    = mVertexBuffer.get(),
                .size          = vertexSize,
                .srcOffset     = 0,
                .dstOffset     = 0,
                .commandBuffer = commandBuffer,
            });

            strandDescStaging->copy({
                .pDstBuffer    = mStrandDescriptionsBuffer.get(),
                .size          = strandDescSize,
                .srcOffset     = 0,
                .dstOffset     = 0,
                .commandBuffer = commandBuffer,
            });
        });

        mBufferAddresses = {
            .vertexBuffer = mVertexBuffer->getAddress(),
            .strandDescriptionsBuffer = mStrandDescriptionsBuffer->getAddress(),
        };
    }

    void HairModel::render(const vk::CommandBuffer& commandBuffer) const
    {

        commandBuffer.drawMeshTasksEXT(mGroupSize, 1, 1);
    }

}