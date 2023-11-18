#ifndef VULKAN_TYPES_HDR
#define VULKAN_TYPES_HDR

#include <optional>

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

struct SwapChainSupportDetails 
{
    VkSurfaceCapabilitiesKHR capabilities = {};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentMode = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentMode, nullptr);

    if (presentMode != 0)
    {
        details.presentModes.resize(presentMode);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentMode, details.presentModes.data());
    }

    return details;
}

} //TED


namespace Gust 
{

struct AllocatedBuffer
{
    VkBuffer buffer;
    VkDeviceMemory bufferMemory;
};

// struct AllocatedImage 
// {
    // VkImage image;
    // VmaAllocation allocation;
// };

}

#endif // !VULKAN_TYPES_HDR
