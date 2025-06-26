#pragma once

#include <cstdint>
#include <string>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include "Util.hpp"

namespace nbl
{
    class Buffer;
    class Device;
    class Image;

    enum class BufferType
    {
        Index,
        Vertex,
        Indirect,
        Storage,
        Uniform,
        AccelerationStructure,
        ShaderBindingTable,
        Staging,
    };

    std::string toString(BufferType bufferType) noexcept;

    struct BufferCreateInfo
    {
        uint64_t        size      = 0;
        BufferType      type      = BufferType::Storage;
        Device*         pDevice   = nullptr;
        std::string     debugName = "Unknown Buffer";
    };

    struct BufferCopyInfo
    {
        Buffer*           pDstBuffer    = nullptr;
        uint64_t          size          = 0;
        uint64_t          srcOffset     = 0;
        uint64_t          dstOffset     = 0;
        vk::CommandBuffer commandBuffer = nullptr;
    };

    struct BufferImageCopyInfo
    {
        Image*            pDstImage     = nullptr;
        vk::CommandBuffer commandBuffer = nullptr;
    };

    class Buffer
    {
    public:
        nbl_DISABLE_COPY(Buffer);
        nbl_CI_CTOR(Buffer, BufferCreateInfo);

        ~Buffer();

        void setData(const void* pData, uint64_t size, uint64_t offset = 0) const;

        void readBack(void* pData, uint64_t size, uint64_t offset = 0) const;

        void copy(const BufferCopyInfo& copyInfo) const;

        void imageCopy(const BufferImageCopyInfo& imageCopyInfo) const;

        const vk::Buffer& getHandle()    const { return mBuffer;              }
        uint64_t          getSize()      const { return mCreateSize;          }
        BufferType        getType()      const { return mBufferType;          }
        uint64_t          getAllocSize() const { return mAllocationInfo.size; }
        uint64_t          getAddress()   const { return mDeviceAddress;       }

    private:
        static vk::BufferUsageFlags getUsageFlags(BufferType bufferType);

        static int32_t getMemoryFlags(BufferType bufferType);

        vk::Buffer          mBuffer;

        VmaAllocation       mAllocation;
        VmaAllocationInfo   mAllocationInfo;
        vk::DeviceAddress   mDeviceAddress;

        uint64_t            mCreateSize {0};
        BufferType          mBufferType;
        std::string         mName;

        Device*             mDevice;
    };
}