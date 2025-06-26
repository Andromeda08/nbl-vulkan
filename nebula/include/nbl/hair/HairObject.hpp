#pragma once

#include <string>
#include <glm/glm.hpp>
#include "math/Transform.hpp"

namespace nbl
{
    class HairModel;

    struct HairObject
    {
        HairModel*  pHairModel = nullptr;
        Transform   transform  = {};
        std::string name       = {};
        glm::vec4   diffuse    = { 0.32549f, 0.23921f, 0.20784f, 1.0f }; // rgb(83, 61, 53)
        glm::vec4   specular   = { 0.41568f, 0.30588f, 0.21960f, 1.0f }; // rgb(106, 78, 56)
    };
}