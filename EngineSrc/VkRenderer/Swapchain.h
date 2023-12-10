#ifndef SWAPCHIAN_HDR
#define SWAPCHIAN_HDR

#include <vector>
#include <string>

#include "vulkan/vulkan.h"

#include "RenderGlobals.h"

namespace Gust 
{
class SwapChain 
{
public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    SwapChain(VkExtent2D windowExtents);
    SwapChain(VkExtent2D windowExtents, SwapChain* previous);
    ~SwapChain();

    SwapChain(const SwapChain& rhs) = delete;
    SwapChain& operator=(const SwapChain& rhs) = delete;

    VkFramebuffer getFramebuffer(int index) const;
    VkRenderPass getRenderPass() const;
    VkImageView getImageView(int index) const;
    size_t getImageCount() const;
    VkFormat getSwapChainImageFormat() const;
    VkExtent2D getSwapChainExtent() const;
    uint32_t getWidth() const;
    uint32_t getHeight() const;
    float getExtentAspectRatio() const;

    VkFormat findDepthFormat();

    VkResult acquireNextImage(uint32_t *imageIndex);
    VkResult submitCommandBuffers(const VkCommandBuffer *buffer, uint32_t *imageIndex);
    bool compareSwapFormats(const SwapChain& swapChain) const;

private:
    void init();
    void createSwapchain();
    void createImageViews();
    void createDepthResources();
    void createRenderPass();
    void createFramebuffers();
    void createSyncObjects();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> & availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    VkFormat _swapChainImageFormat;
    VkFormat _swapChainDepthFormat;
    VkExtent2D _swapChainExtent;

    std::vector<VkFramebuffer> _swapChainFramebuffers;
    VkRenderPass _renderPass;

    std::vector<VkImage> _depthImages;
    std::vector<VkDeviceMemory> _depthImageMemories;
    std::vector<VkImageView> _depthImageViews;
    std::vector<VkImage> _swapChainImages;
    std::vector<VkImageView> _swapChainImageViews;

    VkExtent2D _windowExtent;

    VkSwapchainKHR _swapChain;

    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;
    std::vector<VkFence> _inFlightFences;
    std::vector<VkFence> _imagesInFlight;

    size_t _currentFrame = 0;

    SwapChain* _oldSwapChain;
};

}//GUST

#endif // !SWAPCHIAN_HDR
