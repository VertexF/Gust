#ifndef RENDERING_ENGINE_HDR
#define RENDERING_ENGINE_HDR

#include <vector>
#include <optional>
#include <unordered_map>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Vertex.h"
#include "Core/TimeStep.h"
#include "Mesh.h"
#include "Entity/StaticEntity.h"
#include "Material.h"

namespace
{


} //TED


namespace Gust 
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

//This idea of this class is to start abstracting away some of the code.
//I'm not 100% sure this is what I even what I want to do but have 2 vulkan renders is just going to make a mess.
class Renderer 
{
public:
    Renderer();

    void initVulkan(const char* title);
    void drawFrame(TimeStep timestep, int shaderSwitch = 0);
private:
    void createInstance(const char* title);
    void setupDebugMessenger();

    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();

    void createSwapChain();
    void createImageView();
    void createRenderPass();

    void createColourResources();
    void createDepthResources();
    void createFramebuffer();

    void createCommands();
    bool loadShaderModule(const char *path, VkShaderModule *outShaderModule);
    void createPipeline();
    void createSyncStructures();

    void loadMeshes();
    void createScene();
    void uploadMesh(Mesh &mesh);
    void drawWorld(VkCommandBuffer command, Entity *first, int count);

    void recreateSwapChain();

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
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
                      VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer sourceBuffer, VkBuffer destBuffer, VkDeviceSize size);

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlagBits aspectFlags, uint32_t mipLevels);
    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
                    VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                    VkImage& image, VkDeviceMemory& imageMemory);

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommand(VkCommandBuffer commandBuffer);

    Material* createMaterial(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name);
    Material* getMaterial(const std::string& name);
    Mesh* getMesh(const std::string& name);

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
    VkPipeline _redPipeline;
    VkPipeline _colourPipeline;

    VkCommandPool _commandPool;

    VkImage _textureImage;
    VkDeviceMemory _textureImageMemory;
    VkImageView _textureImageView;
    VkSampler _textureSampler;

    VkBuffer _vertexBuffer;
    VkDeviceMemory _vertexBufferMemory;
    VkBuffer _indexBuffer;
    VkDeviceMemory _indexBufferMemory;
    std::vector<VkBuffer> _uniformBuffers;
    std::vector<VkDeviceMemory> _uniformBufferMemory;
    std::vector<void*> _uniformBufferMapped;

    VkDescriptorPool _descriptorPool;
    std::vector<VkDescriptorSet> _descriptorSets;

    std::vector<VkCommandBuffer> _commandBuffers;

    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;
    std::vector<VkFence> _inFlightFence;

    VmaAllocator _allocator;

    //Vulkan setting data
    VkSampleCountFlagBits _msaaSamples;
    uint32_t _mipLevels;
    uint32_t _currentFrame;

    //GeometryData
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;
    float _time;
    float _flashTime;

    //EntityData
    std::vector<Entity> _renderables;

    std::unordered_map<std::string, Material> _materials;
    std::unordered_map<std::string, Mesh> _meshes;
};
}

#endif //RENDERING_ENGINE_HDR