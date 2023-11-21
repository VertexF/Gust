#ifndef VULKAN_HDR
#define VULAKN_HDR

#include <optional>
#include <vector>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Vertex.h"
#include "RenderGlobals.h"
#include "VulkanTypes.h"

#include "Core/TimeStep.h"

namespace Gust 
{

//The plan is with this class is have everything in it to render a square, then refactor everything afterwards.
// All to make sure I'm not breaking things after things are moved.
//TODO: Refactor class.
class Vulkan 
{
public:
    Vulkan(const char* title);

    void waitDevice();
    void recreateSwapChain();
    void drawFrame(TimeStep timestep);

private:
    void initVulkan(const char* title);

    void createSwapChain();
    void createImageView();
    void createRenderPass();
    void createColourResources();
    void createDepthResources();
    void createFramebuffer();
    void createDescriptionSetLayout();
    void createGraphicsPipeline();
    void createTextureImage();
    void createTextureImageView();
    void createTextureSampler();
    void loadModel();
    void createGeometry();
    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    void createCommandBuffer();
    void createSyncObjects();

    void swapChainCleanUp();

    VkFormat findDepthFormat();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& avaiablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

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

    void copyBuffer(VkBuffer sourceBuffer, VkBuffer destBuffer, VkDeviceSize size);

    void updateUniformBuffers(uint32_t currentImage, TimeStep timestep);
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    static std::vector<char> readFile(const std::string &filename);

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

    //Vulkan setting data
    VkSampleCountFlagBits _msaaSamples;
    uint32_t _mipLevels;
    uint32_t _currentFrame;

    //GeometryData
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;
    float _time;
    float _flashTime;

    //TEST
    QueueFamilyIndices _queuefamilyIndices;
};

} //GUST

#endif // !VULKAN_HDR
