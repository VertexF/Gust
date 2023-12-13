#ifndef RENDER_SYSTEM_HDR
#define RENDER_SYSTEM_HDR

#include "VkRenderer/RenderGlobals.h"
#include "VkRenderer/GameObject.h"
#include "VkRenderer/Pipeline.h"
#include "VkRenderer/Camera.h"
#include "VkRenderer/FrameInfo.h"

// std
#include <memory>
#include <vector>

namespace Gust 
{

class RenderSystem 
{
public:
    RenderSystem();
    ~RenderSystem();

    RenderSystem(const RenderSystem&) = delete;
    RenderSystem& operator=(const RenderSystem&) = delete;

    void init(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);

    void renderGameObjects(FrameInfo &frameInfo);

private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void createPipeline(VkRenderPass renderPass);

    Pipeline *_pipeline;
    VkPipelineLayout _pipelineLayout;
};
}//Gust

#endif // !RENDER_SYSTEM_HDR
