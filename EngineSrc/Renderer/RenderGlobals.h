#ifndef RENDER_GLOBALS_HDR
#define RENDER_GLOBALS_HDR

#include <vector>

#include "VulkanQueueFamilies.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities = {};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

namespace Gust
{
class RenderGlobals
{
public:
    static RenderGlobals& getInstance();

    VkSurfaceKHR getSurface() const;
    VkPhysicalDevice getPhysicalDevice() const;
    VkDevice getLogicalDevice() const;
    VkQueue getGraphicsQueue() const;
    VkQueue getPresentQueue() const;
    QueueFamilyIndices getFamilyIndices() const;
    VkSampleCountFlagBits getMSAASamples() const;

private:
    RenderGlobals();

    void createInstance();

    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();

    bool checkValidateLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    VkSampleCountFlagBits getMaxUsableSampleCount();
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkResult createDebugUtilMessagerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* createInfo,
                                        const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* debugMessenger);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                                                        void* userData);

    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debugMessenger;
    VkSurfaceKHR _surface;

    VkPhysicalDevice _physicalDevice;
    VkDevice _logicalDevice;

    VkQueue _graphicsQueue;
    VkQueue _presentQueue;

    QueueFamilyIndices _queuefamilyIndices;
    VkSampleCountFlagBits _msaaSamples;

public:
    RenderGlobals(RenderGlobals const& rhs) = delete;
    RenderGlobals(RenderGlobals&& rhs) = delete;
    void operator=(RenderGlobals const& rhs) = delete;
    RenderGlobals operator=(RenderGlobals&& rhs) = delete;
};
} //GUST

#endif // !RENDER_GLOBALS_HDR
