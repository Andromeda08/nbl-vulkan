#pragma once

#include <cstdint>
#include <vector>

namespace vk
{
    class Instance;
    class SurfaceKHR;
}

namespace nbl
{
    struct Size2D
    {
        uint32_t width  = 0;
        uint32_t height = 0;
    };

    /**
     * Interface for windowing system integration
     */
    class IWindow
    {
    public:
        virtual ~IWindow() = default;

        /**
         * @return Value indicating whether the window will close or not.
         */
        virtual bool willClose() const = 0;

        /**
         * @return The dimensions of the windows framebuffer.
         */
        virtual Size2D getFramebufferSize() const = 0;

        /**
         * Get all Vulkan extensions required by the windowing system.
         * @return List of extensions
         */
        virtual std::vector<const char*> getVulkanInstanceExtensions() = 0;

        /**
         * Create Vulkan surface for the current window.
         */
        virtual void createVulkanSurface(const vk::Instance& instance, vk::SurfaceKHR* pSurface) = 0;
    };
}