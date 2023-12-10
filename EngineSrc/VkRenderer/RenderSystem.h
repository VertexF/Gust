#ifndef RENDER_SYSTEM_HDR
#define RENDER_SYSTEM_HDR

#include "RenderGlobals.h"
#include "GameObject.h"
#include "Pipeline.h"
#include "Camera.h"
#include "FrameInfo.h"

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

    void renderGameObjects(FrameInfo &frameInfo, std::vector<GameObject>& gameObjects);

private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void createPipeline(VkRenderPass renderPass);

    Pipeline *_pipeline;
    VkPipelineLayout _pipelineLayout;
};
}//Gust

#endif // !RENDER_SYSTEM_HDR
