#ifndef VULKAN_HDR
#define VULAKN_HDR

#include <optional>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Vertex.h"

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

struct SwapChainSupportDetails 
{
    VkSurfaceCapabilitiesKHR capabilities = {};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

} //TED

namespace Gust 
{

//The plan is with this class is have everything in it to render a square, then refactor everything afterwards.
// All to make sure I'm not breaking things after things are moved.
//TODO: Refactor class.
//TODO: Render a square.
class Vulkan 
{
public:
    Vulkan(const char* title, GLFWwindow* window);

private:
    void initVulkan(const char* title, GLFWwindow* window);

    void createInstance(const char* title);
    void setupDebugMessenger();
    void createSurface(GLFWwindow* window);
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain(GLFWwindow* window);
    void createImageView();
    void createRenderPass();

    bool checkValidateLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    VkSampleCountFlagBits getMaxUsableSampleCount();
    VkFormat findDepthFormat();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& avaiablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlagBits aspectFlags, uint32_t mipLevels);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* callbackData, 
                                                        void* userData);

    //VulkanData
    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debugMessenger;
    VkSurfaceKHR _surface;
    
    VkPhysicalDevice _physicalDevice;
    VkDevice _logicalDevice;

    VkQueue _graphicsQueue;
    VkQueue _presentQueue;

    VkSwapchainKHR _swapChain;
    std::vector<VkImage> _swapChainImages;
    VkFormat _swapChainImageFormat;
    VkExtent2D _swapChainExtent;
    std::vector<VkImageView> _swapChainImageView;

    //Vulkan setting data
    VkSampleCountFlagBits _msaaSamples;

    //GeometryData
    std::vector<Vertex> _vertices;
};

} //GUST

#endif // !VULKAN_HDR
