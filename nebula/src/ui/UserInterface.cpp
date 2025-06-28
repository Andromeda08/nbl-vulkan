#include "ui/UserInterface.hpp"

#include <imgui.h>

namespace nbl
{
    UserInterface::UserInterface(const UserInterfaceCreateInfo& createInfo)
    {
        // mImGuiRenderer = ImGuiRenderer::createImGuiRenderer({
        //     .fontPath = createInfo.fontPath,
        //     .pWindow = createInfo.pWindow,
        // });
    }

    void UserInterface::update()
    {
        for (auto&& component : mComponents)
        {
            component->update();
        }
    }

    void UserInterface::draw(const CommandList* commandList, const Frame& currentFrame) const
    {
        // mImGuiRenderer->renderImGui(commandList->handle(), currentFrame, [&]() {
        //     for (auto&& component : mComponents)
        //     {
        //         component->draw();
        //     }
        // });
    }

    void UserInterface::addComponent(std::unique_ptr<UIComponent>&& component)
    {
        mComponents.push_back(std::move(component));
    }

    bool UserInterface::wantCaptureMouse() const noexcept
    {
        const ImGuiIO& io = ImGui::GetIO();
        return io.WantCaptureMouse;
    }

    bool UserInterface::wantCaptureKeyboard() const noexcept
    {
        const ImGuiIO& io = ImGui::GetIO();
        return io.WantCaptureKeyboard;
    }
}
