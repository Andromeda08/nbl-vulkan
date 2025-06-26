#include "Extensions.hpp"

#include "Util.hpp"

namespace nbl
{
    struct VulkanAnyStruct
    {
        vk::StructureType sType;
        const void*       pNext;
    };

    VulkanDeviceExtension::VulkanDeviceExtension(
        const char* extensionName,
        const bool  requested)
    : mExtensionName(extensionName)
    , mIsRequested(requested)
    {
    }

    VulkanDeviceExtension::VulkanDeviceExtension(
        const char*                  extensionName,
        const std::function<void()>& structInitFn,
        const bool                   requested)
    : mExtensionName(extensionName)
    , mIsRequested(requested)
    , mStructInitFn(structInitFn)
    {
        mIsCoreFeatureStruct = std::string(mExtensionName).contains("VulkanCore");
    }

    void VulkanDeviceExtension::postSupportCheck()
    {
        if (!shouldActivate()) return;
        mIsEnabled = true;
        mStructInitFn();
    }

    void VulkanDeviceExtension::preCreateDevice(vk::DeviceCreateInfo& deviceCreateInfo) const
    {
        if (!shouldActivate()) return;
        if (mFeatureStructPtr != nullptr)
        {
            addToNextChain(deviceCreateInfo, mFeatureStructPtr);
        }
    }

    void VulkanDeviceExtension::addToNextChain(vk::DeviceCreateInfo& deviceCreateInfo, void* featureInfo)
    {
        auto* featureStruct  = static_cast<VulkanAnyStruct*>(featureInfo);
        featureStruct->pNext = deviceCreateInfo.pNext;
        deviceCreateInfo.setPNext(featureInfo);
    }

    #define def_VulkanExt(NAME, strEXT_NAME, tSTRUCT, FN)                       \
    class Vulkan##NAME : public VulkanDeviceExtension {                         \
    public:                                                                     \
        Vulkan##NAME() : VulkanDeviceExtension(strEXT_NAME, FN, true) {         \
            mFeatureStructPtr = &mFeatureStruct;                                \
        }                                                                       \
        ~Vulkan##NAME() override = default;                                     \
    private: tSTRUCT m##FeatureStruct;                                          \
    }

    #pragma region "vk::PhysicalDeviceVulkan1(x)Features"

    def_VulkanExt(Core11, "VulkanCore1.1", vk::PhysicalDeviceVulkan11Features, [&]() -> void {
        mFeatureStruct = vk::PhysicalDeviceVulkan11Features();
    });

    def_VulkanExt(Core12, "VulkanCore1.2", vk::PhysicalDeviceVulkan12Features, [&]() -> void {
        mFeatureStruct = vk::PhysicalDeviceVulkan12Features()
            .setBufferDeviceAddress(true)
            .setDescriptorIndexing(true)
            .setScalarBlockLayout(true)
            .setShaderInt8(true)
            .setTimelineSemaphore(true)
            .setHostQueryReset(true)
            .setScalarBlockLayout(true)
            .setDrawIndirectCount(true);
    });

    def_VulkanExt(Core13, "VulkanCore1.3", vk::PhysicalDeviceVulkan13Features, [&]() -> void {
        mFeatureStruct = vk::PhysicalDeviceVulkan13Features()
            .setMaintenance4(true)
            .setDynamicRendering(true)
            .setSynchronization2(true)
            .setInlineUniformBlock(true);
    });

    def_VulkanExt(Core14, "VulkanCore1.4", vk::PhysicalDeviceVulkan14Features, [&]() -> void {
        mFeatureStruct = vk::PhysicalDeviceVulkan14Features()
            .setHostImageCopy(true);
    });

    #pragma endregion

    #pragma region "Device Extensions with Feature Structs"

    // VK_KHR_acceleration_structure
    def_VulkanExt(
        AccelerationStructureExt,
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        vk::PhysicalDeviceAccelerationStructureFeaturesKHR,
        [&]() -> void {
            mFeatureStruct = vk::PhysicalDeviceAccelerationStructureFeaturesKHR()
                .setAccelerationStructure(true);
        }
    );

    // VK_KHR_ray_tracing_pipeline
    def_VulkanExt(
        RayTracingPipelineExt,
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
        vk::PhysicalDeviceRayTracingPipelineFeaturesKHR,
        [&]() -> void {
            mFeatureStruct = vk::PhysicalDeviceRayTracingPipelineFeaturesKHR()
                .setRayTracingPipeline(true);
        }
    );

    // VK_KHR_ray_query
    def_VulkanExt(
        RayQueryExt,
        VK_KHR_RAY_QUERY_EXTENSION_NAME,
        vk::PhysicalDeviceRayQueryFeaturesKHR,
        [&]() -> void {
            mFeatureStruct = vk::PhysicalDeviceRayQueryFeaturesKHR()
                .setRayQuery(true);
        }
    );

    // VK_EXT_mesh_shader
    def_VulkanExt(
        MeshShaderExt,
        VK_EXT_MESH_SHADER_EXTENSION_NAME,
        vk::PhysicalDeviceMeshShaderFeaturesEXT,
        [&]() -> void {
            mFeatureStruct = vk::PhysicalDeviceMeshShaderFeaturesEXT()
                .setMeshShader(true)
                .setTaskShader(true)
                .setMeshShaderQueries(true);
        }
    );

    #pragma endregion

    #undef def_VulkanExt

    std::vector<std::unique_ptr<VulkanDeviceExtension>> VulkanDeviceExtension::getRHIDeviceExtensions(const std::optional<vk::PhysicalDevice> physicalDevice)
    {
        std::vector<std::unique_ptr<VulkanDeviceExtension>> extensions = {};

        extensions.push_back(std::make_unique<VulkanDeviceExtension>(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, true));
        extensions.push_back(std::make_unique<VulkanDeviceExtension>(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME, true));
        extensions.push_back(std::make_unique<VulkanDeviceExtension>(VK_KHR_SWAPCHAIN_EXTENSION_NAME, true));
        extensions.push_back(std::make_unique<VulkanCore11>());
        extensions.push_back(std::make_unique<VulkanCore12>());
        extensions.push_back(std::make_unique<VulkanCore13>());
        extensions.push_back(std::make_unique<VulkanCore14>());
        extensions.push_back(std::make_unique<VulkanAccelerationStructureExt>());
        extensions.push_back(std::make_unique<VulkanRayTracingPipelineExt>());
        extensions.push_back(std::make_unique<VulkanRayQueryExt>());
        extensions.push_back(std::make_unique<VulkanMeshShaderExt>());

        if (!physicalDevice.has_value())
        {
            return extensions;
        }

        const vk::PhysicalDevice gpu = physicalDevice.value();
        const std::vector<vk::ExtensionProperties> availableExtensions = gpu.enumerateDeviceExtensionProperties();

        for (const auto& extension : extensions)
        {
            if (!extension->mIsCoreFeatureStruct && findExtension(extension->getExtensionName(), availableExtensions))
            {
                extension->setSupported();
                if (extension->isRequested())
                {
                    extension->setEnabled();
                }
            }

            if (extension->mIsCoreFeatureStruct)
            {
                extension->setSupported();
                extension->setEnabled();
            }
        }

        return extensions;
    }
}
