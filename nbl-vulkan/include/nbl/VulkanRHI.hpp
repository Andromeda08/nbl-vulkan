#pragma once

#include <vulkan/vulkan.hpp>

#include "Buffer.hpp"
#include "CommandQueue.hpp"
#include "Descriptor.hpp"
#include "Device.hpp"
#include "Frame.hpp"
#include "Image.hpp"
#include "Swapchain.hpp"
#include "Util.hpp"

namespace nbl
{
    class IWindow;

    struct VulkanRHIConfiguration
    {
        bool        validation      = false;
        uint32_t    backBufferCount = 2;
        std::string applicationName = "Unknown Application";
        std::string engineName      = "nbl::VulkanRHI";
    };

    struct VulkanRHICreateInfo
    {
        IWindow*                pWindow       = nullptr;
        VulkanRHIConfiguration  configuration = {};
    };

    /**
     * Vulkan Rendering Hardware Interface
     * [Only one active instance can exist at a time]
     */
    class VulkanRHI
    {
        // Technically this class conforms to a very specific general RHI interface definition.
    public:
        nbl_DISABLE_COPY(VulkanRHI);
        nbl_CI_CTOR(VulkanRHI, VulkanRHICreateInfo);

        // ================================
        // "Frames"
        // ================================

        /**
         * Acquire next Swapchain image and begin the rendering frame.
         * @return Current Frame and Acquired Image Indices
         */
        Frame beginFrame() const;

        /**
         * Submit current frame with recorded command buffers to the GPU,
         * present to Swapchain and advance frame counter.
         */
        void submitFrame(const Frame& frame);

        // ================================
        // Resource Creation
        // ================================

        /**
         * Create a new Buffer resource with the given parameters.
         * @return Buffer
         */
        std::unique_ptr<Buffer> createBuffer(const BufferCreateInfo& createInfo) const;

        /**
         * Create a new Descriptor resource with the given parameters.
         * @return Descriptor
         */
        std::unique_ptr<Descriptor> createDescriptor(const DescriptorCreateInfo& createInfo) const;

        /**
         * Create a new Image resource with the given parameters.
         * @return Image
         */
        std::unique_ptr<Image> createImage(const ImageCreateInfo& createInfo) const;

        // ================================
        // Getter Methods for GPU Objects
        // ================================

        Device*       getDevice()        const { return mDevice.get();        }
        CommandQueue* getGraphicsQueue() const { return mGraphicsQueue.get(); }
        CommandQueue* getComputeQueue()  const { return mComputeQueue.get();  }
        Swapchain*    getSwapchain()     const { return mSwapchain.get();     }

    private:
        void createInstance();

        static bool                     sExists;

        IWindow*                        mWindow = nullptr;
        const VulkanRHIConfiguration    mConfig = {};

        vk::Instance                    mInstance;
        std::vector<const char*>        mInstanceLayers;
        std::vector<const char*>        mInstanceExtensions;

        std::unique_ptr<Device>         mDevice;

        std::unique_ptr<CommandQueue>   mGraphicsQueue;
        std::unique_ptr<CommandQueue>   mComputeQueue;
        std::unique_ptr<Swapchain>      mSwapchain;

        uint32_t                        mBackBufferCount = 2;
        uint32_t                        mCurrentFrame    = 0;

        std::vector<vk::Fence>          mFrameInFlight;
        std::vector<vk::Semaphore>      mImageReady;
        std::vector<vk::Semaphore>      mRenderingFinished;
    };
}