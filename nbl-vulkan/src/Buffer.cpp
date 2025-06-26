#include "Buffer.hpp"

#include "Device.hpp"
#include "Image.hpp"

namespace nbl
{
    std::string toString(const BufferType bufferType) noexcept
    {
        switch (bufferType)
        {
            case BufferType::Index:                 return "Index";
            case BufferType::Vertex:                return "Vertex";
            case BufferType::Indirect:              return "Indirect";
            case BufferType::Storage:               return "Storage";
            case BufferType::Uniform:               return "Uniform";
            case BufferType::AccelerationStructure: return "AccelerationStructure";
            case BufferType::ShaderBindingTable:    return "ShaderBindingTable";
            case BufferType::Staging:               return "Staging";
            default:                                return "Unknown";
        }
    }
    
    Buffer::Buffer(const BufferCreateInfo& createInfo)
    : mCreateSize(createInfo.size)
    , mBufferType(createInfo.type)
    , mName(createInfo.debugName)
    , mDevice(createInfo.pDevice)
    {
        auto bufferInfo = vk::BufferCreateInfo()
            .setSize(createInfo.size)
            .setUsage(getUsageFlags(mBufferType));
    
        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags = getMemoryFlags(mBufferType);
    
        if (mBufferType == BufferType::Staging)
        {
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        }
    
        const auto* pBufferInfo = reinterpret_cast<VkBufferCreateInfo*>(&bufferInfo);
        auto* pBuffer = reinterpret_cast<VkBuffer*>(&mBuffer);
        nbl_VK_C_RESULT(
            vmaCreateBuffer(mDevice->getAllocator(), pBufferInfo, &allocInfo,
                pBuffer, &mAllocation, &mAllocationInfo)
        );
    
        mDevice->nameObject<vk::Buffer>({
            .debugName = mName,
            .handle = mBuffer,
        });
    
        const auto addressInfo = vk::BufferDeviceAddressInfo()
            .setBuffer(mBuffer);
        nbl_VK_TRY(mDeviceAddress = mDevice->getHandle().getBufferAddress(&addressInfo);)
    }
    
    Buffer::~Buffer()
    {
        vmaDestroyBuffer(mDevice->getAllocator(), mBuffer, mAllocation);
    }
    
    void Buffer::setData(const void* pData, const uint64_t size, const uint64_t offset) const
    {
        nbl_VK_C_RESULT(
            vmaCopyMemoryToAllocation(mDevice->getAllocator(), pData, mAllocation, offset, size));
    }
    
    void Buffer::readBack(void* pData, const uint64_t size, const uint64_t offset) const
    {
        nbl_VK_C_RESULT(
            vmaCopyAllocationToMemory(mDevice->getAllocator(), mAllocation, offset, pData, size));
    }
    
    void Buffer::copy(const BufferCopyInfo& copyInfo) const
    {
        const auto copyRegion = vk::BufferCopy()
            .setSize(copyInfo.size)
            .setSrcOffset(copyInfo.srcOffset)
            .setDstOffset(copyInfo.dstOffset);
        copyInfo.commandBuffer.copyBuffer(mBuffer, copyInfo.pDstBuffer->mBuffer, 1, &copyRegion);
    }
    
    void Buffer::imageCopy(const BufferImageCopyInfo& imageCopyInfo) const
    {
        const auto imgProps = imageCopyInfo.pDstImage->getProperties();
        const auto copyRegion = vk::BufferImageCopy()
            .setBufferOffset(0)
            .setBufferRowLength(0)
            .setBufferImageHeight(0)
            .setImageSubresource(imgProps.subresourceLayers)
            .setImageOffset({0, 0, 0})
            .setImageExtent({imgProps.extent.width, imgProps.extent.height, 1});
        imageCopyInfo.commandBuffer.copyBufferToImage(mBuffer, imageCopyInfo.pDstImage->getImage(), vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion);
    }
    
    vk::BufferUsageFlags Buffer::getUsageFlags(const BufferType bufferType)
    {
        using enum vk::BufferUsageFlagBits;
        vk::BufferUsageFlags result = eTransferSrc | eTransferDst | eShaderDeviceAddress;
    
        switch (bufferType)
        {
            case BufferType::Index:{
                result |= eIndexBuffer
                    | eStorageBuffer
                    | eAccelerationStructureBuildInputReadOnlyKHR;
                break;
            }
            case BufferType::Vertex:{
                result |= eVertexBuffer
                    | eStorageBuffer
                    | eAccelerationStructureBuildInputReadOnlyKHR;
                break;
            }
            case BufferType::Indirect:{
                result |= eIndirectBuffer;
                break;
            }
            case BufferType::Storage: {
                result |= eStorageBuffer | eAccelerationStructureBuildInputReadOnlyKHR;
                break;
            }
            case BufferType::Uniform: {
                result |= eUniformBuffer;
                break;
            }
            case BufferType::AccelerationStructure: {
                result |= eAccelerationStructureStorageKHR;
                break;
            }
            case BufferType::ShaderBindingTable: {
                result |= eShaderBindingTableKHR;
                break;
            }
            case BufferType::Staging: {
                break;
            }
        }
    
        return result;
    }
    
    int32_t Buffer::getMemoryFlags(const BufferType bufferType)
    {
        switch (bufferType)
        {
            case BufferType::Uniform:
            case BufferType::Staging:
                return VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                       | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT
                       | VMA_ALLOCATION_CREATE_MAPPED_BIT;
            default:
                return 0;
        }
    }
}