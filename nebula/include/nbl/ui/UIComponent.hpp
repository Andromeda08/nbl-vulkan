#pragma once

#include "Util.hpp"

namespace nbl
{
    class UIComponent
    {
    public:
        nbl_DISABLE_COPY(UIComponent);

        UIComponent() = default;
        virtual ~UIComponent() = default;

        virtual void update() = 0;
        virtual void draw()   = 0;
    };
}