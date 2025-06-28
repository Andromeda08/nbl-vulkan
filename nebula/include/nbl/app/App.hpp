#pragma once

#include <memory>
#include <nbl/VulkanRHI.hpp>
#include <wsi/Window.hpp>

#include "camera/FirstPersonCamera.hpp"
#include "hair/HairModel.hpp"
#include "hair/HairPipeline.hpp"
#include "ui/UserInterface.hpp"

namespace nbl
{
    struct AppCreateInfo
    {
        wsi::WindowCreateInfo   windowInfo    = {};
        VulkanRHIConfiguration  rhiInfo       = {};
        bool                    enableUI      = true;
    };

    class App
    {
    public:
        nbl_CI_CTOR(App, AppCreateInfo);

        virtual ~App() = default;

        virtual void run();

    private:
        void createCameraResources();
        void loadHairModels();

        std::unique_ptr<wsi::Window>            mWindow;
        std::unique_ptr<VulkanRHI>              mRHI;
        std::unique_ptr<UserInterface>          mUI;

        std::unique_ptr<FirstPersonCamera>      mCamera;

        std::array<std::unique_ptr<Buffer>, 2>  mUniformBuffer;
        std::unique_ptr<Descriptor>             mSceneDescriptor;

        std::vector<std::unique_ptr<HairModel>> mHairModels;
        HairModel*                              mActiveHairModel;

        std::unique_ptr<HairPipeline>           mHairPipeline;
    };
}
