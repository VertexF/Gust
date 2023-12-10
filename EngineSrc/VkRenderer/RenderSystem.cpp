#include "RenderSystem.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <cassert>
#include <stdexcept>

#include "Core/Logger.h"

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
    createPipelineLayout(globalSetLayout);
    createPipeline(renderPass);
}

void RenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
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
    GUST_CORE_ASSERT(_pipelineLayout == nullptr, "Cannot create pipeline before pipeline layoutn.");

    PipelineConfigInfo pipelineConfig;
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = _pipelineLayout;
    _pipeline = new Pipeline("Assets/Shaders/simple.vert.spv", "Assets/Shaders/simple.frag.spv", pipelineConfig);
}

void RenderSystem::renderGameObjects(FrameInfo& frameInfo, std::vector<GameObject>& gameObjects)
{
    _pipeline->bind(frameInfo.commandBuffer);

    vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
                            _pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

    for (auto& obj : gameObjects)
    {
        //obj.transform.rotation.y = glm::mod(obj.transform.rotation.y + 0.001f, glm::two_pi<float>());
        //obj.transform.rotation.x = glm::mod(obj.transform.rotation.x + 0.0005f, glm::two_pi<float>());
        //obj.transform.rotation.z = glm::mod(obj.transform.rotation.z + 0.0015f, glm::two_pi<float>());

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