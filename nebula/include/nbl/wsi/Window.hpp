#pragma once

#include <memory>
#include <string>
#include <GLFW/glfw3.h>
#include <nbl/IWindow.hpp>
#include "Util.hpp"

namespace nbl::wsi
{
    enum class WindowResolutionPreset
    {
        None,
        w1280_h720,
        w1600_h900,
        w1920_h1080,
        w2560_h1440,
    };

    Size2D resolutionPresetToSize(WindowResolutionPreset preset);

    /**
     * Resolution setting precedence:
     * 1. Automatic resolution
     * 2. Resolution preset
     * 3. Manual width & height
     */
    struct WindowCreateInfo
    {
        uint32_t               width            = 1280;
        uint32_t               height           = 720;
        std::string            title            = "Unknown Nebula Window";
        bool                   autoResolution   = false;
        WindowResolutionPreset resolutionPreset = WindowResolutionPreset::None;
    };

    /**
     * GLFW-based Window
     */
    class Window final : public IWindow
    {
    public:
        nbl_DISABLE_COPY(Window);
        nbl_CI_CTOR(Window, WindowCreateInfo);

        float getUIScaleFactor() const;

        GLFWwindow* getHandle() const { return mWindow; }

        ~Window() override;

        bool willClose() const override;

        Size2D getFramebufferSize() const override;

        std::vector<const char*> getVulkanInstanceExtensions() override;

        void createVulkanSurface(const vk::Instance& instance, vk::SurfaceKHR* pSurface) override;

    private:
        static void defaultKeyHandler(GLFWwindow* window, int key, int scancode, int action, int mods);

        GLFWwindow* mWindow = nullptr;

        Size2D      mFramebufferSize;

        std::string mTitle;
        uint32_t    mWidth;
        uint32_t    mHeight;
    };
}