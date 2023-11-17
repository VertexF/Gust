#ifndef VULKAN_QUEUE_FAMILIES
#define VULKAN_QUEUE_FAMILIES

#include <optional>
#include <vector>
#include "vulkan/vulkan.h"

namespace 
{

struct QueueFamilyIndices
{

std::optional<uint32_t> graphicsFamily;
std::optional<uint32_t> presentFamily;

bool isComplete()
{
    return graphicsFamily.has_value() && presentFamily.has_value();
}

};

struct QueueFamily
{

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamilies : queueFamilies)
    {
        if (queueFamilies.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport)
        {
            indices.presentFamily = i;
        }

        if (indices.isComplete())
        {
            break;
        }

        i++;
    }

    return indices;
}
};

} //TED

#endif // !VULKAN_QUEUE_FAMILIES
