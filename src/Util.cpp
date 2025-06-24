#include "Util.hpp"

#include <ranges>

namespace nbl
{
    int32_t findExtension(const char* extensionName, const std::vector<vk::ExtensionProperties>& extensionProperties)
    {
        if (extensionName == nullptr)
        {
            return gInvalidIndex;
        }

        const auto it = std::ranges::find_if(extensionProperties, [extensionName](const vk::ExtensionProperties& extProps) -> bool {
            return !std::strcmp(extProps.extensionName, extensionName);
        });

        return (it != std::end(extensionProperties))
            ? static_cast<int32_t>(std::distance(std::begin(extensionProperties), it))
            : gInvalidIndex;
    }

    int32_t findLayer(const char* layerName, const std::vector<vk::LayerProperties>& layerProperties)
    {
        if (layerName == nullptr)
        {
            return gInvalidIndex;
        }

        const auto it = std::ranges::find_if(layerProperties, [layerName](const vk::LayerProperties& layerProps) -> bool {
            return !std::strcmp(layerProps.layerName, layerName);
        });

        return (it != std::end(layerProperties))
            ? static_cast<int32_t>(std::distance(std::begin(layerProperties), it))
            : gInvalidIndex;
    }

    std::vector<const char*> getSupportedExtensions(
        const std::vector<const char*>&             requestedExtensions,
        const std::vector<vk::ExtensionProperties>& extensionsProperties)
    {
        std::set uniqueExtensions(std::begin(requestedExtensions), std::end(requestedExtensions));
        auto supportedExtensions = uniqueExtensions | std::views::filter([&](const char* value) -> bool {
            return std::ranges::find_if(extensionsProperties, [&](const vk::ExtensionProperties& extProps) -> int {
                return std::strcmp(value, extProps.extensionName);
            }) != std::end(extensionsProperties);
        });
        return { std::begin(supportedExtensions), std::end(supportedExtensions) };
    }

    std::vector<const char*> getSupportedLayers(
        const std::vector<const char*>&         requestedLayers,
        const std::vector<vk::LayerProperties>& layerProperties)
    {
        std::set uniqueLayers(std::begin(requestedLayers), std::end(requestedLayers));
        auto supportedLayers = uniqueLayers | std::views::filter([&](const char* value) -> bool {
            return std::ranges::find_if(layerProperties, [&](const vk::LayerProperties& layerProps) -> int {
                return std::strcmp(value, layerProps.layerName);
            }) != std::end(layerProperties);
        });
        return { std::begin(supportedLayers), std::end(supportedLayers) };
    }

    std::optional<QueueProperties> findQueue(
        const vk::PhysicalDevice  physicalDevice,
        const vk::QueueFlags      requiredFlags,
        const vk::QueueFlags      excludedFlags,
        const std::set<uint32_t>& excludedQueueFamilies)
    {
        for (const std::vector queueFamilies = physicalDevice.getQueueFamilyProperties();
            auto&& [familyIndex, properties] : std::views::enumerate(queueFamilies))
        {
            if ((properties.queueCount > 0)
                && (properties.queueFlags & requiredFlags)
                && !(properties.queueFlags & excludedFlags)
                && !excludedQueueFamilies.contains(familyIndex))
            {
                QueueProperties queueProperties = { properties, static_cast<uint32_t>(familyIndex) };
                return std::make_optional(queueProperties);
            }
        }

        return std::nullopt;
    }
}
