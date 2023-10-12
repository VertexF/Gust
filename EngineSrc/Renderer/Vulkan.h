#ifndef VULKAN_HDR
#define VULAKN_HDR

#include <optional>
#include <vector>
#include <string>

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

    void waitDevice();
    void recreateSwapChain(GLFWwindow* window);

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
    void createColourResources();
    void createDepthResources();
    void createFramebuffer();
    void createDescriptionSetLayout();
    void createGraphicsPipeline();
    void createCommandPool();
    void createTextureImage();

    void swapChainCleanUp();

    bool checkValidateLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    VkSampleCountFlagBits getMaxUsableSampleCount();

    VkFormat findDepthFormat();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& avaiablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, 
                      VkDeviceMemory &bufferMemory);

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlagBits aspectFlags, uint32_t mipLevels);
    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, 
                     VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, 
                     VkImage &image, VkDeviceMemory &imageMemory);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevel);

    VkShaderModule createShaderModule(const std::vector<char> &code);

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommand(VkCommandBuffer commandBuffer);
    void copyBuffer(VkBuffer sourceBuffer, VkBuffer destBuffer, VkDeviceSize size);

    static std::vector<char> readFile(const std::string &filename);
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

    VkRenderPass _renderPass;

    VkImage _colourImage;
    VkDeviceMemory _colourImageMemory;
    VkImageView _colourImageView;

    VkImage _depthImage;
    VkDeviceMemory _depthImageMemory;
    VkImageView _depthImageView;

    std::vector<VkFramebuffer> _swapChainFramebuffers;
    VkDescriptorSetLayout _descriptorSetLayout;

    VkPipelineLayout _pipelineLayout;
    VkPipeline _graphicsPipeline;

    VkCommandPool _commandPool;

    VkImage _textureImage;
    VkDeviceMemory _textureImageMemory;

    //Vulkan setting data
    VkSampleCountFlagBits _msaaSamples;
    uint32_t _mipLevels;

    //GeometryData
    std::vector<Vertex> _vertices;
};

} //GUST

#endif // !VULKAN_HDR
