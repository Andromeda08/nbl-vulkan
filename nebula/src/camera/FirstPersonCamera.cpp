#include "camera/FirstPersonCamera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace nbl
{
    FirstPersonCamera::FirstPersonCamera(glm::ivec2 size, glm::vec3 eye, float h_fov, float near, float far)
    : ICamera(), mSize(size), mEye(eye), mNear(near), mFar(far), mFov(h_fov) {}

    glm::mat4 FirstPersonCamera::view() const
    {
        return glm::lookAt(mEye, mEye + mOrientation, mUp);
    }

    glm::mat4 FirstPersonCamera::projection() const
    {
        return glm::perspective(
            glm::radians(mFov),
            static_cast<float>(mSize.x) / static_cast<float>(mSize.y),
            mNear, mFar);
    }

    void FirstPersonCamera::registerKeys(GLFWwindow* pWindow)
    {
        // WASD movement
        if (glfwGetKey(pWindow, GLFW_KEY_W) == GLFW_PRESS)
        {
            mEye += mSpeed * mOrientation;
        }
        if (glfwGetKey(pWindow, GLFW_KEY_A) == GLFW_PRESS)
        {
            mEye += mSpeed * -glm::normalize(glm::cross(mOrientation, mUp));
        }
        if (glfwGetKey(pWindow, GLFW_KEY_S) == GLFW_PRESS)
        {
            mEye += mSpeed * -mOrientation;
        }
        if (glfwGetKey(pWindow, GLFW_KEY_D) == GLFW_PRESS)
        {
            mEye += mSpeed * glm::normalize(glm::cross(mOrientation, mUp));
        }

        // Move up & down
        if (glfwGetKey(pWindow, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            mEye += (mSpeed / 2.0f) * mUp;
        }
        if (glfwGetKey(pWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        {
            mEye -= (mSpeed / 2.0f) * mUp;
        }

        // Exit on ESC
        if (glfwGetKey(pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(pWindow, true);
        }
    }

    void FirstPersonCamera::registerMouse(GLFWwindow* pWindow)
    {
        if (glfwGetMouseButton(pWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

            if (mClick)
            {
                // glfwSetCursorPos(pWindow, (mSize.x / 2), (mSize.y / 2));
                glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                mClick = false;
            }

            double mouseX, mouseY;
            glfwGetCursorPos(pWindow, &mouseX, &mouseY);

            float rotX = mSensitivity * (float)(mouseY - (mSize.y / 2)) / mSize.y;
            float rotY = mSensitivity * (float)(mouseX - (mSize.x / 2)) / mSize.x;

            glm::vec3 newOrientation = glm::rotate(mOrientation, glm::radians(-rotX), glm::normalize(glm::cross(mOrientation, mUp)));

            if (abs(glm::angle(newOrientation, mUp) - glm::radians(90.0f)) <= glm::radians(85.0f))
            {
                mOrientation = newOrientation;
            }

            mOrientation = glm::rotate(mOrientation, glm::radians(-rotY), mUp);
            glfwSetCursorPos(pWindow, (mSize.x / 2), (mSize.y / 2));
        }
        else if (glfwGetMouseButton(pWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
        {
            glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            mClick = true;
        }
    }

    CameraData FirstPersonCamera::getCameraData() const
    {
        auto v = view();
        auto p = projection();
        auto e = eye();

        return {
            .view = v,
            .proj = p,
            .viewInverse = glm::inverse(v),
            .projInverse = glm::inverse(p),
            .eye = { e.x, e.y, e.z, 1.0f },
            .nearPlane = mNear,
            .farPlane = mFar,
        };
    }
}