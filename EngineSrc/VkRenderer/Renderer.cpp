#include "Renderer.h"

#include "Core/Logger.h"
#include "Core/Global.h"
#include "Core/Instrumentor.h"

namespace Gust
{


Renderer::Renderer() : _swapchain(nullptr), _currentFrameIndex(0), _currentImageIndex(0)
{
    GUST_PROFILE_FUNCTION();
    recreateSwapchain();
    createCommandBuffers();
}

Renderer::~Renderer()
{
    freeCommandBuffers();

    delete _swapchain;
}

VkRenderPass Renderer::getSwapChainRenderPass() const
{
    return _swapchain->getRenderPass();
}

float Renderer::getAspectRatio() const
{
    return (_swapchain->getExtentAspectRatio());
}

bool Renderer::isFrameInProgress() const
{
    return _isFrameStarted;
}

VkCommandBuffer Renderer::getCurrentCommandBuffer() const
{
    GUST_CORE_ASSERT(_isFrameStarted == false, "You need to start drawing before you get the current command buffer.");

    return _commandBuffers[_currentFrameIndex];
}

int Renderer::getFrameIndex() const
{
    GUST_CORE_ASSERT(_isFrameStarted == false, "You need to start drawing before you get the current command buffer.");

    return _currentFrameIndex;
}

VkCommandBuffer Renderer::beginFrame() 
{
    GUST_PROFILE_FUNCTION();
    GUST_CORE_ASSERT(_isFrameStarted, "You can not begin beginFrame while already in progress.");

    VkResult result = _swapchain->acquireNextImage(&_currentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapchain();
        return nullptr;
    }
    GUST_CORE_ASSERT(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR, "Could not acquire swapchain image.");

    _isFrameStarted = true;
    VkCommandBuffer commandBuffer = getCurrentCommandBuffer();

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Could not begin recording command buffer.");

    return commandBuffer;
}

void Renderer::endFrame() 
{
    GUST_PROFILE_FUNCTION();
    GUST_CORE_ASSERT(_isFrameStarted == false, "You can not end frame while a frame is in progress.");

    VkCommandBuffer commandBuffer = getCurrentCommandBuffer();

    VkResult result = vkEndCommandBuffer(commandBuffer);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Could not record command buffer.");

    result = _swapchain->submitCommandBuffers(&commandBuffer, &_currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        recreateSwapchain();
    }

    GUST_CORE_ASSERT(result != VK_SUCCESS, "Could not present swapchain image.");
    _isFrameStarted = false;
    _currentFrameIndex = (_currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) 
{
    GUST_PROFILE_FUNCTION();
    GUST_CORE_ASSERT(_isFrameStarted == false, "You can not call beginSwapChainRenderPass if a frame is not in progress.");
    GUST_CORE_ASSERT(commandBuffer != getCurrentCommandBuffer(), "You can not begin render pass on a command buffer from a different frame.");

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _swapchain->getRenderPass();
    renderPassInfo.framebuffer = _swapchain->getFramebuffer(_currentImageIndex);
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = _swapchain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.f };
    clearValues[1].depthStencil = { 1.f, 0 };
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = static_cast<float>(_swapchain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(_swapchain->getSwapChainExtent().height);
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;
    VkRect2D scissor = { {0, 0}, _swapchain->getSwapChainExtent() };

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) 
{
    GUST_PROFILE_FUNCTION();
    GUST_CORE_ASSERT(_isFrameStarted == false, "You can not call endSwapChainRenderPass if a frame is not in progress.");
    GUST_CORE_ASSERT(commandBuffer != getCurrentCommandBuffer(), "You can not begin render pass on a command buffer from a different frame.");

    vkCmdEndRenderPass(commandBuffer);
}

void Renderer::createCommandBuffers()
{
    GUST_PROFILE_FUNCTION();
    _commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandPool = RenderGlobals::getInstance().getDevice()->getCommandPool();
    allocateInfo.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size());

    VkResult result = vkAllocateCommandBuffers(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), &allocateInfo, _commandBuffers.data());
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Could not allocate command buffer.");
}

void Renderer::freeCommandBuffers()
{
    GUST_PROFILE_FUNCTION();
    vkFreeCommandBuffers(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), RenderGlobals::getInstance().getDevice()->getCommandPool(),
        static_cast<uint32_t>(_commandBuffers.size()), _commandBuffers.data());

    _commandBuffers.clear();
}

void Renderer::recreateSwapchain()
{
    GUST_PROFILE_FUNCTION();
    VkExtent2D extents = Global::getInstance().getWindowExtent();
    while (extents.height == 0 || extents.width == 0)
    {
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(RenderGlobals::getInstance().getDevice()->getLogicalDevice());

    if (_swapchain)
    {

        _swapchain = new SwapChain(Global::getInstance().getWindowExtent(), _swapchain);

        //GUST_CORE_ASSERT(oldSwapChain != _swapchain, "Swap chain image or depth format has changed.");
        //delete oldSwapChain;
        //oldSwapChain = nullptr;
    }
    else
    {
        _swapchain = new SwapChain(Global::getInstance().getWindowExtent());
    }
}

}//Gust