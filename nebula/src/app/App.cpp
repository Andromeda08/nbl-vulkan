#include "app/App.hpp"

#include <fmt/format.h>

#include "Barrier.hpp"

namespace nbl
{
    App::App(const AppCreateInfo& createInfo)
    {
        mWindow = wsi::Window::createWindow(createInfo.windowInfo);

        mRHI = VulkanRHI::createVulkanRHI({
            .pWindow       = mWindow.get(),
            .configuration = createInfo.rhiInfo,
        });

        // mUI = UserInterface::createUserInterface({
        //     .fontPath = "JetBrainsMono-Regular.ttf",
        //     .pWindow  = mWindow.get(),
        // });

        createCameraResources();

        loadHairModels();

        mHairPipeline = std::make_unique<HairPipeline>(mRHI.get(), mSceneDescriptor.get());
    }

    void App::run()
    {
        while (!mWindow->willClose())
        {
            glfwPollEvents();

            // if (!mUI->wantCaptureKeyboard())
            // {
                mCamera->registerKeys(mWindow->getHandle());
            // }
            // if (!mUI->wantCaptureMouse())
            // {
                mCamera->registerMouse(mWindow->getHandle());
            // }

            Frame frameInfo = mRHI->beginFrame();
            const uint32_t currentFrame = frameInfo.currentFrame;
            auto* commandList = mRHI->getGraphicsQueue()->getCommandList(currentFrame);

            // mUI->update();

            const auto cameraData = mCamera->getCameraData();
            mUniformBuffer[currentFrame]->setData(&cameraData, sizeof(CameraData), 0);

            commandList->begin();
            {
                mRHI->getSwapchain()->setScissorViewport(commandList->handle());

                mHairPipeline->renderHairModel(mActiveHairModel, commandList, frameInfo);

                Barrier::transitionImageLayout({
                    .commandBuffer = commandList->handle(),
                    .imageTransitionInfo = {
                        .pImage    = mRHI->getSwapchain()->getImage(frameInfo.acquiredImageIndex),
                        .newLayout = vk::ImageLayout::ePresentSrcKHR,
                    },
                });
            }
            commandList->end();

            frameInfo.addCommandLists({ commandList->handle() });

            mRHI->submitFrame(frameInfo);
        }
    }

    void App::createCameraResources()
    {
        const auto extent = mRHI->getSwapchain()->getExtent();
        mCamera = std::make_unique<FirstPersonCamera>(
            glm::ivec2{ extent.width, extent.height },
            glm::vec3(-17.0f, 16.0f, 144.0f));

        const auto cameraData = mCamera->getCameraData();
        for (uint32_t i = 0; i < mRHI->getSwapchain()->getImageCount(); i++)
        {
            mUniformBuffer[i] = mRHI->createBuffer({
                .size      = sizeof(CameraData),
                .type      = BufferType::Uniform,
                .debugName = fmt::format("Camera UB {}", i),
            });
            mUniformBuffer[i]->setData(&cameraData, sizeof(CameraData), 0);
        }

        vk::ShaderStageFlags stageFlags;
        {
            using enum vk::ShaderStageFlagBits;
            stageFlags = eFragment | eVertex |  eTaskEXT |  eMeshEXT;
        }

        const std::vector sceneDescriptorBindings = {
            vk::DescriptorSetLayoutBinding().setBinding(0).setDescriptorType(vk::DescriptorType::eUniformBuffer).setStageFlags(stageFlags).setDescriptorCount(1),
        };

        mSceneDescriptor = mRHI->createDescriptor({
            .bindings = sceneDescriptorBindings,
            .setCount = mRHI->getSwapchain()->getImageCount(),
            .debugName = "Scene Descriptor",
        });

        auto cameraInfo = vk::DescriptorBufferInfo()
            .setOffset(0)
            .setRange(sizeof(CameraData));

        for (uint32_t i = 0; i < mSceneDescriptor->getSetCount(); i++)
        {
            cameraInfo.setBuffer(mUniformBuffer[i]->getHandle());
            const auto write = DescriptorWriteInfo()
                .setSetIndex(i)
                .writeUniformBuffers(0, 1, &cameraInfo);

            mSceneDescriptor->write(write);
        }
    }

    void App::loadHairModels()
    {
        static std::vector hairModels = {
            /*"wCurly.hair", "wStraight.hair", "wWavy.hair", "wWavyThin.hair"*/
            "wWavy.hair"
        };

        for (const char* model : hairModels)
        {
            mHairModels.push_back(HairModel::createHairModel({
                .filePath = model,
                .pRHI     = mRHI.get(),
            }));
        }

        mActiveHairModel = mHairModels[0].get();
    }
}
