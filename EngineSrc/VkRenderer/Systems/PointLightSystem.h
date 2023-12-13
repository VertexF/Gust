#ifndef POINT_LIGHT_SYSTEM_HDR
#define POINT_LIGHT_SYSTEM_HDR

#include <vector>

#include "VkRenderer/Camera.h"
#include "VkRenderer/FrameInfo.h"
#include "VkRenderer/GameObject.h"
#include "VkRenderer/Pipeline.h"

namespace Gust
{
class PointLightSystem 
{
public:
    PointLightSystem();
    ~PointLightSystem();

    PointLightSystem(const PointLightSystem& rhs) = delete;
    PointLightSystem& operator=(const PointLightSystem& rhs) = delete;

    void init(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);

    void update(FrameInfo& frameInfo, GlobalUBO& ubo);
    void render(FrameInfo &frameInfo);
private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void createPipeline(VkRenderPass renderPass);

    Pipeline* _pipeline;
    VkPipelineLayout _pipelineLayout;
};
}//Gust

#endif // !POINT_LIGHT_SYSTEM_HDR
