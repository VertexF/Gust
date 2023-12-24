#ifndef DEVICE_HDR
#define DEVICE_HDR

#include <string>
#include <vector>

//Needed to make the GLEW linking is linking to the static version.
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Core/Global.h"

namespace Gust 
{
struct SwapChainSupportDetails 
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices 
{
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool graphicsFamilyHasValue = false;
    bool presentFamilyHasValue = false;
    bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
};

class Device 
{
public:
#ifdef GUST_DEBUG
    const bool enableValidationLayers = true;
#else
    const bool enableValidationLayers = false;
#endif // GUST_DEBUG
public:
    Device();
    ~Device();

    Device(const Device &rhs) = delete;
    Device& operator=(const Device& rhs) = delete;
    Device(Device && rhs) = delete;
    Device &operator=(Device && rhs) = delete;

    VkCommandPool getCommandPool() const;
    VkDevice getLogicalDevice() const;
    VkSurfaceKHR getSurface() const;
    VkQueue getGraphicsQueue() const;
    VkQueue getPresentQueue() const;

    SwapChainSupportDetails getSwapChainSupport();
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    QueueFamilyIndices findPhysicalDeviceFamilies();
    VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    void createBuffer(VkDeviceSize deviceSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void copyBuffer(VkBuffer sourceBuffer, VkBuffer destBuffer, VkDeviceSize deviceSize);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);
    void createImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);

    VkPhysicalDeviceProperties deviceProperties;

private:
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();

    bool isDeviceSuitable(VkPhysicalDevice device);
    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo);
    void hasGLFWRequiredInstanceExtensions();
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    VkInstance _instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice _physcialDevice = VK_NULL_HANDLE;
    VkCommandPool _commandPool;

    VkDevice _logicalDevice;
    VkSurfaceKHR _surface;
    VkQueue _graphicQueue;
    VkQueue _presentQueue;

    const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
};
}//GUST

#endif // !DEVICE_HDR
