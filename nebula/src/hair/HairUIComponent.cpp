#include "hair/HairUIComponent.hpp"

#include <vector>

#include <imgui.h>
#include <fmt/format.h>
#include <glm/gtc/type_ptr.hpp>

#include "hair/HairModel.hpp"


namespace nbl
{
    HairUIComponent::HairUIComponent(HairModel* hairObject)
    : UIComponent()
    , mHairModel(hairObject)
    {
        mComponentName = fmt::format("{} Control", mHairModel->mName);
    }

    void HairUIComponent::draw()
    {
        static std::vector renderingModes = {
            HairRenderingMode::Normal,       HairRenderingMode::DebugQuads,
            HairRenderingMode::DebugStrands, HairRenderingMode::DebugStrandlets,
        };

        ImGui::Begin(mComponentName.c_str());
        {
            ImGui::SliderFloat3("Diffuse", glm::value_ptr(mHairModel->mDiffuse), 0.0f, 1.0f);
            ImGui::SliderFloat3("Specular", glm::value_ptr(mHairModel->mSpecular), 0.0f, 1.0f);

            ImGui::Separator();

            ImGui::SliderFloat3("Scale", glm::value_ptr(mHairModel->mTransform.scale), 0.0f, 1.0f);
            ImGui::SliderFloat3("Euler", glm::value_ptr(mHairModel->mTransform.euler), -180.0f, 180.0f);

            ImGui::Separator();
            
            ImGui::BeginCombo(
                "Render Mode",
                toString(mHairModel->mRenderingMode).c_str());
            {
                for (const auto mode : renderingModes)
                {
                    if (ImGui::Selectable(toString(mode).c_str(), mHairModel->mRenderingMode == mode))
                    {
                        mHairModel->mRenderingMode = mode;
                    }
                }
            }
            ImGui::EndCombo();

            ImGui::Separator();

            ImGui::Checkbox("Enable Group Size Override", &mHairModel->mEnableOverride);
            ImGui::SliderInt(
                "Override Group Size",
                &mHairModel->mGroupSizeOverride,
                0,
                static_cast<int32_t>(mHairModel->mGroupSize));
        }
        ImGui::End();
    }
}
