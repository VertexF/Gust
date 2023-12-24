#include "Swapchain.h"

#include <array>
#include <cstdlib>
#include <limits>
#include <set>

#include "Core/Logger.h"
#include "Core/Instrumentor.h"

namespace Gust 
{
SwapChain::SwapChain(VkExtent2D windowExtents) : _windowExtent(windowExtents), _oldSwapChain(nullptr)
{
    init();
}

SwapChain::SwapChain(VkExtent2D windowExtents, SwapChain* previous) : 
    _windowExtent(windowExtents), _oldSwapChain(previous)
{
    init();
    delete _oldSwapChain;
    _oldSwapChain = nullptr;
}

SwapChain::~SwapChain() 
{
    for (auto imageView : _swapChainImageViews) 
    {
        vkDestroyImageView(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), imageView, nullptr);
    }
    _swapChainImageViews.clear();

    if (_swapChain != nullptr) 
    {
        vkDestroySwapchainKHR(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), _swapChain, nullptr);
    }

    for (int i = 0; i < _depthImages.size(); i++) 
    {
        vkDestroyImageView(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), _depthImageViews[i], nullptr);
        vkDestroyImage(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), _depthImages[i], nullptr);
        vkFreeMemory(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), _depthImageMemories[i], nullptr);
    }

    for (auto framebuffer : _swapChainFramebuffers)
    {
        vkDestroyFramebuffer(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), framebuffer, nullptr);
    }

    vkDestroyRenderPass(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), _renderPass, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
    {
        vkDestroySemaphore(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), _renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), _imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), _inFlightFences[i], nullptr);
    }
}

VkFramebuffer SwapChain::getFramebuffer(int index) const 
{
    return _swapChainFramebuffers[index];
}

VkRenderPass SwapChain::getRenderPass() const 
{
    return _renderPass;
}

VkImageView SwapChain::getImageView(int index) const 
{
    return _swapChainImageViews[index];
}

size_t SwapChain::getImageCount() const 
{
    return _swapChainImages.size();
}

VkFormat SwapChain::getSwapChainImageFormat() const 
{
    return _swapChainImageFormat;
}

VkExtent2D SwapChain::getSwapChainExtent() const 
{
    return _swapChainExtent;
}

uint32_t SwapChain::getWidth() const 
{
    return _swapChainExtent.width;
}

uint32_t SwapChain::getHeight() const 
{

    return _swapChainExtent.height;
}

float SwapChain::getExtentAspectRatio() const 
{
    return (static_cast<float>(_swapChainExtent.width) / static_cast<float>(_swapChainExtent.height));
}

VkFormat SwapChain::findDepthFormat() 
{
    return RenderGlobals::getInstance().getDevice()->findSupportedFormat
    (
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

VkResult SwapChain::acquireNextImage(uint32_t* imageIndex) 
{
    vkWaitForFences(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), 1, &_inFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), _swapChain, UINT64_MAX,
                                            _imageAvailableSemaphores[_currentFrame], VK_NULL_HANDLE, imageIndex);

    return result;
}

VkResult SwapChain::submitCommandBuffers(const VkCommandBuffer* buffer, uint32_t* imageIndex) 
{
    if (_imagesInFlight[*imageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), 1, &_imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
    }
    _imagesInFlight[*imageIndex] = _inFlightFences[_currentFrame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { _imageAvailableSemaphores[_currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = buffer;

    VkSemaphore signalSemaphores[] = { _renderFinishedSemaphores[_currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), 1, &_inFlightFences[_currentFrame]);
    VkResult result = vkQueueSubmit(RenderGlobals::getInstance().getDevice()->getGraphicsQueue(), 1, &submitInfo, _inFlightFences[_currentFrame]);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Could not submit command buffer to the graphics queue.");

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { _swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = imageIndex;
    //NOTE: we take care of the erroring of function in the drawFrame function.
    vkQueuePresentKHR(RenderGlobals::getInstance().getDevice()->getPresentQueue(), &presentInfo);

    _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return result;
}

bool SwapChain::compareSwapFormats(const SwapChain& swapChain) const
{
    return (swapChain._swapChainDepthFormat == _swapChainDepthFormat && swapChain._swapChainImageFormat == _swapChainImageFormat);
}

void SwapChain::init() 
{
    GUST_PROFILE_FUNCTION();
    createSwapchain();
    createImageViews();
    createRenderPass();
    createDepthResources();
    createFramebuffers();
    createSyncObjects();
}

void SwapChain::createSwapchain() 
{
    GUST_PROFILE_FUNCTION();
    SwapChainSupportDetails swapChainSupport = RenderGlobals::getInstance().getDevice()->getSwapChainSupport();

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && 
        imageCount > swapChainSupport.capabilities.maxImageCount > 0 && 
        imageCount > swapChainSupport.capabilities.maxImageCount) 
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = RenderGlobals::getInstance().getDevice()->getSurface();
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = RenderGlobals::getInstance().getDevice()->findPhysicalDeviceFamilies();
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

    if (indices.graphicsFamily != indices.presentFamily) 
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else 
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = _oldSwapChain == nullptr ? VK_NULL_HANDLE : _oldSwapChain->_swapChain;

    VkResult result = vkCreateSwapchainKHR(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), &createInfo, nullptr, &_swapChain);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Could not create a swapchain.");

    vkGetSwapchainImagesKHR(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), _swapChain, &imageCount, nullptr);
    _swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), _swapChain, &imageCount, _swapChainImages.data());

    _swapChainImageFormat = surfaceFormat.format;
    _swapChainExtent = extent;
}

void SwapChain::createImageViews()
{
    GUST_PROFILE_FUNCTION();
    _swapChainImageViews.resize(_swapChainImages.size());

    for (size_t i = 0; i < _swapChainImages.size(); i++) 
    {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = _swapChainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = _swapChainImageFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkResult result = vkCreateImageView(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), &viewInfo, nullptr, &_swapChainImageViews[i]);
        GUST_CORE_ASSERT(result != VK_SUCCESS, "Could not create image view.");
    }
}

void SwapChain::createDepthResources() 
{
    GUST_PROFILE_FUNCTION();
    VkFormat depthFormat = findDepthFormat();
    _swapChainDepthFormat = depthFormat;
    VkExtent2D swapChainExtent = getSwapChainExtent();

    _depthImages.resize(getImageCount());
    _depthImageMemories.resize(getImageCount());
    _depthImageViews.resize(getImageCount());

    for (int i = 0; i < _depthImages.size(); i++) 
    {
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = _swapChainExtent.width;
        imageInfo.extent.height = _swapChainExtent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = depthFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = 0;

        RenderGlobals::getInstance().getDevice()->createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _depthImages[i], _depthImageMemories[i]);

        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = _depthImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = depthFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkResult result = vkCreateImageView(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), &viewInfo, nullptr, &_depthImageViews[i]);
        GUST_CORE_ASSERT(result != VK_SUCCESS, "Could not create image view.");
    }
}

void SwapChain::createRenderPass()
{
    GUST_PROFILE_FUNCTION();
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colourAttachment = {};
    colourAttachment.format = getSwapChainImageFormat();
    colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colourAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colourAttachmentRef = {};
    colourAttachmentRef.attachment = 0;
    colourAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colourAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.dstSubpass = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    std::array<VkAttachmentDescription, 2> attachments = { colourAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VkResult result = vkCreateRenderPass(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), &renderPassInfo, nullptr, &_renderPass);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Could not create render pass.");
}

void SwapChain::createFramebuffers()
{
    GUST_PROFILE_FUNCTION();
    _swapChainFramebuffers.resize(getImageCount());

    for (size_t i = 0; i < getImageCount(); i++) 
    {
        std::array<VkImageView, 2> attachment = { _swapChainImageViews[i], _depthImageViews[i] };

        VkExtent2D swapChainExtent = getSwapChainExtent();

        VkFramebufferCreateInfo framebufferCreateInfo = {};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = _renderPass;
        framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachment.size());
        framebufferCreateInfo.pAttachments = attachment.data();
        framebufferCreateInfo.width = _swapChainExtent.width;
        framebufferCreateInfo.height = _swapChainExtent.height;
        framebufferCreateInfo.layers = 1;

        VkResult result = vkCreateFramebuffer(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), &framebufferCreateInfo, nullptr, &_swapChainFramebuffers[i]);
        GUST_CORE_ASSERT(result != VK_SUCCESS, "Could not create framebuffer.");
    }
}

void SwapChain::createSyncObjects()
{
    GUST_PROFILE_FUNCTION();
    _imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    _imagesInFlight.resize(getImageCount(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
    {
        VkResult result = vkCreateSemaphore(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]);
        GUST_CORE_ASSERT(result != VK_SUCCESS, "Could not create the image available semaphore.");

        result = vkCreateSemaphore(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]);
        GUST_CORE_ASSERT(result != VK_SUCCESS, "Could not create the render finished semaphore.");

        result = vkCreateFence(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), &fenceInfo, nullptr, &_inFlightFences[i]);
        GUST_CORE_ASSERT(result != VK_SUCCESS, "Could not create fence.");
    }
}

VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) 
{
    GUST_PROFILE_FUNCTION();
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }
}

VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) 
{
    GUST_PROFILE_FUNCTION();
    for (const auto &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            GUST_INFO("Present mode mailbox");
            return availablePresentMode;
        }
    }

    GUST_INFO("Present mode V-Sync");
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) 
{
    GUST_PROFILE_FUNCTION();
    if (capabilities.currentExtent.width != UINT64_MAX) 
    {
        return capabilities.currentExtent;
    }
    else 
    {
        VkExtent2D actualExtent = _windowExtent;
        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

}//Gust