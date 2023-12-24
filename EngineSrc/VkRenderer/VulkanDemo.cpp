#include "VulkanDemo.h"

#include <utility>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Core/Logger.h"

namespace Gust 
{

VulkanDemo::VulkanDemo() : _viewerObject(GameObject::createGameObject()), 
    _globalPool(DescriptorPool::Builder().setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
                                         .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
                                         .build())
{
    GUST_PROFILE_FUNCTION();
    for (int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) 
    {
        _globalUniformBuffers.emplace_back(new Buffer(sizeof(GlobalUBO), 1,
                                                      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                      RenderGlobals::getInstance().getDevice()->deviceProperties.limits.minUniformBufferOffsetAlignment));
        _globalUniformBuffers[i]->map();
    }

    _globalSetLayout = DescriptorSetLayout::Builder()
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
        .build();

    for (int i = 0; i < _globalDescriptionSets.size(); i++) 
    {
        auto bufferInfo = _globalUniformBuffers[i]->descriptionInfo();
        DescriptorWriter(*_globalSetLayout, *_globalPool).writeBuffer(0, &bufferInfo).build(_globalDescriptionSets[i]);
    }

    _simpleRenderSystem.init(_renderer.getSwapChainRenderPass(), _globalSetLayout->getDescriptorSetLayout());
    _pointLightSystem.init(_renderer.getSwapChainRenderPass(), _globalSetLayout->getDescriptorSetLayout());
    _viewerObject.transform.translation.z = 2.5f;
}

VulkanDemo::~VulkanDemo()
{
    delete _globalSetLayout;
    delete _globalPool;
}

void VulkanDemo::renderer(TimeStep timestep, std::unordered_map<uint32_t, GameObject>& allGameObjects)
{
    GUST_PROFILE_FUNCTION();
    _camerController.moveInPlaneXZ(timestep, _viewerObject);
    _camera.setViewYXZ(_viewerObject.transform.translation, _viewerObject.transform.rotation);

    float apsect = _renderer.getAspectRatio();
    _camera.setPerspectiveProjection(glm::radians(50.f), apsect, 0.1f, 100.f);

    if (auto commandBuffer = _renderer.beginFrame())
    {
        int frameIndex = _renderer.getFrameIndex();
        FrameInfo frameInfo { frameIndex, timestep, commandBuffer, _camera, _globalDescriptionSets[frameIndex], allGameObjects };

        //Update
        _uniBufferObj.projection = _camera.getProjection();
        _uniBufferObj.view = _camera.getView();
        _uniBufferObj.inverseView = _camera.getInverseView();
        _pointLightSystem.update(frameInfo, _uniBufferObj);
        _globalUniformBuffers[frameIndex]->writeToBuffer(&_uniBufferObj);

        //Render
        _renderer.beginSwapChainRenderPass(commandBuffer);
        _simpleRenderSystem.renderGameObjects(frameInfo);
        _pointLightSystem.render(frameInfo);
        _renderer.endSwapChainRenderPass(commandBuffer);
        _renderer.endFrame();
    }

    vkDeviceWaitIdle(RenderGlobals::getInstance().getDevice()->getLogicalDevice());
}

}//GUST
