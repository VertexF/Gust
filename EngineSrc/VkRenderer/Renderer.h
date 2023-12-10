#ifndef RENDERER_HDR
#define RENDERER_HDR

#include <vector>

#include "Core/Global.h"
#include "Core/TimeStep.h"

#include "RenderGlobals.h"
#include "Swapchain.h"

namespace Gust
{
class Renderer
{
public:
    Renderer();
    ~Renderer();

    Renderer(const Renderer& rhs) = delete;
    Renderer& operator=(const Renderer& rhs) = delete;

    VkRenderPass getSwapChainRenderPass() const;
    float getAspectRatio() const;
    bool isFrameInProgress() const;
    VkCommandBuffer getCurrentCommandBuffer() const;
    int getFrameIndex() const;

    VkCommandBuffer beginFrame();
    void endFrame();

    void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
    void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
private:
    void createCommandBuffers();
    void freeCommandBuffers();
    void recreateSwapchain();

    SwapChain* _swapchain;

    VkPipelineLayout _pipelineLayout;
    std::vector<VkCommandBuffer> _commandBuffers;

    uint32_t _currentImageIndex;
    int _currentFrameIndex;
    bool _isFrameStarted = false;
};
}//GUST

#endif // !RENDERER_HDR
