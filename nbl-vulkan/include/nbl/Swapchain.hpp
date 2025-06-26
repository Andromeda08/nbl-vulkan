#pragma once

#include <cstdint>
#include <vulkan/vulkan.hpp>

#include "IAttachmentSource.hpp"
#include "Util.hpp"

namespace nbl
{
    class Device;
    class Image;
    class IWindow;

    struct SwapchainCreateInfo
    {
        IWindow*       pWindow = nullptr;
        Device*        pDevice = nullptr;
        vk::Instance   instance;
        uint32_t       imageCount {};
    };

    class Swapchain final : public IAttachmentSource
    {
    public:
        nbl_DISABLE_COPY(Swapchain);
        nbl_CI_CTOR(Swapchain, SwapchainCreateInfo);

        ~Swapchain() override;

        void present(vk::Semaphore waitSemaphore, uint32_t imageIndex) const;

        void setScissorViewport(const vk::CommandBuffer& commandList) const;

        vk::SwapchainKHR getHandle()      const          { return mSwapchain;   }

        float            getAspectRatio() const          { return mAspectRatio; }
        uint32_t         getImageCount()  const          { return mImageCount;  }
        vk::Extent2D     getExtent()      const          { return mExtent;      }
        vk::Format       getFormat()      const override { return mFormat;      }

        Image*           getImage(size_t i)     const;
        vk::Image        getVkImage(size_t i)   const;
        vk::ImageView    getImageView(size_t i) const;

        /**
         * @return Image View for the last acquired image.
         */
        vk::ImageView getAttachmentSource() const override;

    private:
        void createSurface();
        void checkSwapchainSupport();
        void createSwapchain();
        void acquireImages();
        void makeDynamicState();

        friend class RenderPass;
        friend class VulkanRHI;
        uint32_t                            mLastAcquiredIndex = 0;

        const uint32_t                      mImageCount;
        vk::Extent2D                        mExtent;
        float                               mAspectRatio {0.0f};
        vk::Format                          mFormat {vk::Format::eB8G8R8A8Unorm};
        vk::ColorSpaceKHR                   mColorSpace {vk::ColorSpaceKHR::eSrgbNonlinear};
        vk::PresentModeKHR                  mPresentMode {vk::PresentModeKHR::eFifo};
        vk::SurfaceTransformFlagBitsKHR     mCurrentTransform {};
        vk::SurfaceKHR                      mSurface;
        vk::SwapchainKHR                    mSwapchain;
        vk::Rect2D                          mCachedScissor;
        vk::Viewport                        mCachedViewport;

        std::vector<vk::Image>              mImages;
        std::vector<vk::ImageView>          mImageViews;

        std::vector<std::unique_ptr<Image>> mWrappedImages;

        IWindow*                            mWindow;
        Device*                             mDevice;
        vk::Instance                        mInstance;
    };
}