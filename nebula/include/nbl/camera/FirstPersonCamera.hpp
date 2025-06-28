#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "ICamera.hpp"

namespace nbl
{
    class FirstPersonCamera final : public ICamera
    {
    public:
        FirstPersonCamera(glm::ivec2 size, glm::vec3 eye, float h_fov = 75.0f, float near = 0.1f, float far = 10000.0f);

        ~FirstPersonCamera() override = default;

        const glm::vec3& eye() const override { return mEye; }

        glm::mat4 view() const override;

        glm::mat4 projection() const override;

        CameraData getCameraData() const override;

        void registerKeys(GLFWwindow* pWindow) override;

        void registerMouse(GLFWwindow* pWindow) override;

    private:
        glm::ivec2 mSize;
        glm::vec3  mEye;
        glm::vec3  mUp = {0, 1, 0};
        glm::vec3  mOrientation = {0, 0, -1};

        float mNear;
        float mFar;
        float mFov;

        float mSpeed {0.25f};
        float mSensitivity {50.0f};

        bool mClick {false};
    };
}