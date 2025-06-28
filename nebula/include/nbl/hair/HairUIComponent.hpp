#pragma once

#include <string>

#include "HairCommon.h"
#include "HairModel.hpp"
#include "ui/UIComponent.hpp"

namespace nbl
{
    class HairUIComponent final : public UIComponent
    {
    public:
        explicit HairUIComponent(HairModel* hairObject);

        ~HairUIComponent() override = default;

        void update() override {}

        void draw() override;

    private:
        HairModel*  mHairModel;
        std::string mComponentName;
    };
}
