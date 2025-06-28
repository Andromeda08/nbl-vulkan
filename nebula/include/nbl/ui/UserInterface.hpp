#pragma once

#include <memory>
#include <vector>
#include <nbl/VulkanRHI.hpp>
#include "UIComponent.hpp"
#include "wsi/Window.hpp"

namespace nbl
{
    struct UserInterfaceCreateInfo
    {
        std::string     fontPath;
        wsi::Window*    pWindow;
    };

    class UserInterface
    {
    public:
        nbl_DISABLE_COPY(UserInterface);
        nbl_CI_CTOR(UserInterface, UserInterfaceCreateInfo);

        void update();

        void draw(const CommandList* commandList, const Frame& currentFrame) const;

        void addComponent(std::unique_ptr<UIComponent>&& component);

        bool wantCaptureMouse() const noexcept;

        bool wantCaptureKeyboard() const noexcept;

    private:
        std::vector<std::unique_ptr<UIComponent>> mComponents;
        // std::unique_ptr<ImGuiRenderer>            mImGuiRenderer;
    };
}