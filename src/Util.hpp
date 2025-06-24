#pragma once

#include <cstdint>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.hpp>
#include "Common.hpp"

namespace nbl
{
    static constexpr int32_t gInvalidIndex = -1;

    struct RHIError final : std::runtime_error
    {
        explicit RHIError(const std::string& msg) : std::runtime_error(msg) {}

        explicit RHIError(const vk::Result result) : std::runtime_error(vk::to_string(result)) {}

        explicit RHIError(const VkResult result) : std::runtime_error(string_VkResult(result)) {}
    };

    // ==============================
    // Code Template Macros
    // ==============================
    #pragma region

    // Delete copy and move operations for a Type.
    #define nbl_DISABLE_COPY(TYPE)              \
        TYPE(const TYPE&) = delete;             \
        TYPE& operator=(const TYPE&) = delete;  \
        TYPE(const TYPE&&) = delete;            \
        TYPE& operator=(const TYPE&&) = delete;

    // Define a constructor with a "CreateInfo" type struct + static uptr creation method.
    #define nbl_CI_CTOR(TYPE, CREATE_INFO_TYPE)                                                 \
        explicit TYPE(const CREATE_INFO_TYPE& createInfo);                                      \
        inline static std::unique_ptr<TYPE> create##TYPE(const CREATE_INFO_TYPE& createInfo) {  \
            return std::make_unique<TYPE>(createInfo);                                          \
        }

    #pragma endregion

    // ==============================
    // Vulkan Error Handling Macros
    // ==============================
    #pragma region

    // Check Vulkan function call exception throwing.
    #define nbl_VK_TRY(TRY_BODY)            \
    try { TRY_BODY }                        \
    catch (const vk::SystemError& err) {    \
        throw;                              \
    }

    // Check Vulkan function call result
    #define nbl_VK_RESULT(RESULT_EXPRESSION)                                        \
    if (const auto result = RESULT_EXPRESSION; result != vk::Result::eSuccess) {    \
        throw RHIError(result);                                                     \
    }

    // For handling VMA results.
    #define nbl_VK_C_RESULT(RESULT_EXPRESSION)                                      \
    if (const auto result = RESULT_EXPRESSION; result != VK_SUCCESS) {              \
         throw RHIError(result);                                                    \
    }

    #pragma endregion

    // ==============================
    // Vulkan Utility Methods
    // ==============================
    template <class Chain_t, class Struct_t>
    void addToPNext(Chain_t& existing, Struct_t& added)
    {
        added.setPNext((void*)(existing.pNext));
        existing.setPNext((void*)(&added));
    }

    /**
     * Find the specified extension by name in a list of Vulkan extension properties.
     * @return Index of extension when found, invalid index otherwise.
     */
    int32_t findExtension(const char* extensionName, const std::vector<vk::ExtensionProperties>& extensionProperties);

    /**
     * Find the specified layer by name in a list of Vulkan layer properties.
     * @return Index of layer when found, invalid index otherwise.
     */
    int32_t findLayer(const char* layerName, const std::vector<vk::LayerProperties>& layerProperties);

    /**
     * Filter a given list of requested extension based on device support.
     * @return Filtered list of extension names.
     */
    std::vector<const char*> getSupportedExtensions(
        const std::vector<const char*>&             requestedExtensions,
        const std::vector<vk::ExtensionProperties>& extensionsProperties);

    /**
     * Filter a given list of requested layer based on device support.
     * @return Filtered list of layer names.
     */
    std::vector<const char*> getSupportedLayers(
        const std::vector<const char*>&         requestedLayers,
        const std::vector<vk::LayerProperties>& layerProperties);

    /**
     * Find a queue given the specific parameters.
     * @return QueueProperties when a queue that satisfies the given parameters was found.
     */
    std::optional<QueueProperties> findQueue(
        vk::PhysicalDevice        physicalDevice,
        vk::QueueFlags            requiredFlags,
        vk::QueueFlags            excludedFlags         = {},
        const std::set<uint32_t>& excludedQueueFamilies = {});
}