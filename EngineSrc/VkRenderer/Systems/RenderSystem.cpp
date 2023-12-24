#include "RenderSystem.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <cassert>
#include <stdexcept>

#include "Core/Logger.h"
#include "Core/Instrumentor.h"

namespace Gust 
{

struct SimplePushConstantData 
{
    glm::mat4 modelMatrix{ 1.f };
    glm::mat4 normalMatrix{ 1.f };
};

RenderSystem::RenderSystem()
{
}

RenderSystem::~RenderSystem() 
{
    vkDestroyPipelineLayout(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), _pipelineLayout, nullptr);

    delete _pipeline;
}

void RenderSystem::init(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
{
    GUST_PROFILE_FUNCTION();
    createPipelineLayout(globalSetLayout);
    createPipeline(renderPass);
}

void RenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
    GUST_PROFILE_FUNCTION();
    VkPushConstantRange pushConstantsRange = {};
    pushConstantsRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantsRange.offset = 0;
    pushConstantsRange.size = sizeof(SimplePushConstantData);

    std::vector<VkDescriptorSetLayout> descriptorSetLayout = { globalSetLayout };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayout.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayout.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantsRange;
    VkResult result = vkCreatePipelineLayout(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), &pipelineLayoutInfo, nullptr, &_pipelineLayout);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Could not create pipeline layout.");
}

void RenderSystem::createPipeline(VkRenderPass renderPass)
{
    GUST_PROFILE_FUNCTION();
    GUST_CORE_ASSERT(_pipelineLayout == nullptr, "Cannot create pipeline before pipeline layoutn.");

    PipelineConfigInfo pipelineConfig;
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = _pipelineLayout;
    _pipeline = new Pipeline("Assets/Shaders/simple.vert.spv", "Assets/Shaders/simple.frag.spv", pipelineConfig);
}

void RenderSystem::renderGameObjects(FrameInfo& frameInfo)
{
    GUST_PROFILE_FUNCTION();
    _pipeline->bind(frameInfo.commandBuffer);

    vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
                            _pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

    for (auto& keyValue : frameInfo.gameObjects)
    {
        auto& obj = keyValue.second;
        if (obj.model == nullptr || ((obj.currentState & State::ACTIVE) == false))
        {
            continue;
        }

        SimplePushConstantData push = {};
        push.modelMatrix =  obj.transform.mat4();
        push.normalMatrix = obj.transform.normalMatrix();

        vkCmdPushConstants(frameInfo.commandBuffer, _pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0, sizeof(SimplePushConstantData), &push);

        obj.model->bind(frameInfo.commandBuffer);
        obj.model->draw(frameInfo.commandBuffer);
    }
}

}//Gust