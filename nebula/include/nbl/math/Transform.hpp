#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace nbl
{
    struct Transform
    {
        glm::vec3 translate = glm::vec3(0.0f);
        glm::vec3 scale     = glm::vec3(1.0f);
        glm::vec3 euler     = glm::vec3(0.0f);

        inline glm::mat4 model() const
        {
            const glm::mat4 T = glm::translate(glm::mat4(1.0f), translate);
            const glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
            const glm::mat4 R = glm::yawPitchRoll(glm::radians(euler.y), glm::radians( euler.x), glm::radians(euler.z));
            return T * R * S;
        }
    };

}