#include "PointLightSystem.h"

#include <array>
#include <map>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"

#include "Core/Logger.h"
#include "VkRenderer/RenderGlobals.h"

namespace Gust 
{

struct PointLightPushConstants 
{
    glm::vec4 position{};
    glm::vec4 colour{};
    float radius;
};

PointLightSystem::PointLightSystem() 
{
}

PointLightSystem::~PointLightSystem() 
{
    vkDestroyPipelineLayout(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), _pipelineLayout, nullptr);

    delete _pipeline;
}

void PointLightSystem::init(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
{
    createPipelineLayout(globalSetLayout);
    createPipeline(renderPass);
}

void PointLightSystem::update(FrameInfo& frameInfo, GlobalUBO& ubo)
{
    auto rotateLight = glm::rotate(glm::mat4(1.f), 0.5f * frameInfo.frameTime, { 0.f, -1.f, 0.f });
    int lightIndex = 0;
    for (auto& keyValue : frameInfo.gameObjects)
    {
        auto& object = keyValue.second;
        if (object.pointLight == nullptr)
        {
            continue;
        }

        GUST_CORE_ASSERT(lightIndex > MAX_LIGHTS, "Current we only support 10 lights.");

        object.transform.translation = glm::vec3(rotateLight * glm::vec4(object.transform.translation, 1.f));

        //Copy the light to the global UBO
        ubo.pointLights[lightIndex].position = glm::vec4(object.transform.translation, 1.f);
        ubo.pointLights[lightIndex].colour = glm::vec4(object.colour, object.pointLight->lightIntensity);

        lightIndex++;
    }

    ubo.numLights = lightIndex;
}

void PointLightSystem::render(FrameInfo& frameInfo) 
{
    std::map<float, uint32_t> sorted;
    for (auto &keyValue : frameInfo.gameObjects) 
    {
        auto& object = keyValue.second;
        if (object.pointLight == nullptr) 
        {
            continue;
        }

        auto offset = frameInfo.camera.getPosition() - object.transform.translation;
        float disSquared = glm::dot(offset, offset);
        sorted[disSquared] = object.getID();
    }

    _pipeline->bind(frameInfo.commandBuffer);

    vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 
                            0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

    for (auto it = sorted.rbegin(); it != sorted.rend(); ++it)
    {
        auto& object = frameInfo.gameObjects.at(it->second);

        PointLightPushConstants push{};
        push.position = glm::vec4(object.transform.translation, 1.f);
        push.colour = glm::vec4(object.colour, object.pointLight->lightIntensity);
        push.radius = object.transform.scale.x;

        vkCmdPushConstants(frameInfo.commandBuffer, _pipelineLayout, 
                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, 
                           sizeof(PointLightPushConstants), &push);

        vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
    }
}

void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) 
{
    VkPushConstantRange pushConstantsrange = {};
    pushConstantsrange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantsrange.offset = 0;
    pushConstantsrange.size = sizeof(PointLightPushConstants);

    std::vector<VkDescriptorSetLayout> descriptorSetLayout{globalSetLayout};

    VkPipelineLayoutCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayout.size());
    pipelineCreateInfo.pSetLayouts = descriptorSetLayout.data();
    pipelineCreateInfo.pushConstantRangeCount = 1;
    pipelineCreateInfo.pPushConstantRanges = &pushConstantsrange;

    VkResult result = vkCreatePipelineLayout(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), 
                                             &pipelineCreateInfo, nullptr, &_pipelineLayout);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Cannot create pipeline layout.");
}

void PointLightSystem::createPipeline(VkRenderPass renderPass) 
{
    GUST_CORE_ASSERT(_pipelineLayout == nullptr, "Cannot create pipeline before pipeline layout.");

    PipelineConfigInfo pipelineConfig = {};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    Pipeline::enableAlphaBlending(pipelineConfig);
    pipelineConfig.attributeDescriptions.clear();
    pipelineConfig.bindingDescriptions.clear();
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = _pipelineLayout;

    _pipeline = new Pipeline("Assets/Shaders/pointLight.vert.spv", "Assets/Shaders/pointLight.frag.spv", pipelineConfig);
}

}//Gust