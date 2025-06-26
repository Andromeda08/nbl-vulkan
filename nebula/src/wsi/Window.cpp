#include "wsi/Window.hpp"

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.hpp>

namespace nbl::wsi
{
    Size2D resolutionPresetToSize(const WindowResolutionPreset preset)
    {
        switch (preset)
        {
            case WindowResolutionPreset::w1280_h720:    return { 1280, 720 };
            case WindowResolutionPreset::w1600_h900:    return { 1600, 900 };
            case WindowResolutionPreset::w1920_h1080:   return { 1920, 1080 };
            case WindowResolutionPreset::w2560_h1440:   return { 2560, 1400 };
            default: {
                throw std::invalid_argument("Invalid resolution preset");
            }
        }
    }

    Window::Window(const WindowCreateInfo& createInfo)
    : mTitle(createInfo.title)
    {
        if (!glfwInit())
        {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, false);

        mWidth  = createInfo.width;
        mHeight = createInfo.height;

        if (createInfo.autoResolution)
        {
            GLFWmonitor* display = glfwGetPrimaryMonitor();
            const GLFWvidmode* videoMode = glfwGetVideoMode(display);

            constexpr float scaleFactor = 0.75f;
            mWidth  = static_cast<uint32_t>(static_cast<float>(videoMode->width)  * scaleFactor);
            mHeight = static_cast<uint32_t>(static_cast<float>(videoMode->height) * scaleFactor);
        }

        if (!createInfo.autoResolution and createInfo.resolutionPreset != WindowResolutionPreset::None)
        {
            auto [w, h] = resolutionPresetToSize(createInfo.resolutionPreset);
            mWidth  = w;
            mHeight = h;
        }

        mWindow = glfwCreateWindow(
            static_cast<int32_t>(mWidth),
            static_cast<int32_t>(mHeight),
            mTitle.c_str(),
            nullptr,
            nullptr
        );

        if (!mWindow)
        {
            throw std::runtime_error("Failed to create Window");
        }

        glfwSetKeyCallback(mWindow, Window::defaultKeyHandler);

        int32_t framebufferWidth;
        int32_t framebufferHeight;
        glfwGetFramebufferSize(mWindow, &framebufferWidth, &framebufferHeight);
        mFramebufferSize = {
            .width  = static_cast<uint32_t>(framebufferWidth),
            .height = static_cast<uint32_t>(framebufferHeight),
        };
    }

    float Window::getUIScaleFactor() const
    {
        if (mWidth <= 2540 && mHeight <= 1440)
        {
            return 1.0f;
        }
        return 1.5f;
    }

    Window::~Window()
    {
        if (mWindow)
        {
            glfwDestroyWindow(mWindow);
        }
        glfwTerminate();
    }

    bool Window::willClose() const
    {
        return glfwWindowShouldClose(mWindow);
    }

    Size2D Window::getFramebufferSize() const
    {
        return mFramebufferSize;
    }

    std::vector<const char*> Window::getVulkanInstanceExtensions()
    {
        uint32_t extensionCount = 0;
        const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
        return {extensions, extensions + extensionCount};
    }

    void Window::createVulkanSurface(const vk::Instance& instance, vk::SurfaceKHR* pSurface)
    {
        if (const VkResult result = glfwCreateWindowSurface(instance, mWindow, nullptr, reinterpret_cast<VkSurfaceKHR*>(pSurface));
            result != VK_SUCCESS)
        {
            throw std::runtime_error(string_VkResult(result));
        }
    }

    void Window::defaultKeyHandler(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }
    }
}