#include "VulkanDemo.h"

#include <memory>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"

#include "Core/Logger.h"
#include "FrameInfo.h"

namespace Gust 
{

VulkanDemo::VulkanDemo() : _viewerObject(GameObject::createGameObject()), 
    _globalPool(DescriptorPool::Builder().setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
                                          .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
                                          .build())
{
    for (int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) 
    {
        _globalUniformBuffers.emplace_back(new Buffer(sizeof(GlobalUBO), 1,
                                                      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                      RenderGlobals::getInstance().getDevice()->properties.limits.minUniformBufferOffsetAlignment));
        _globalUniformBuffers[i]->map();
    }

    _globalSetLayout = DescriptorSetLayout::Builder()
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
        .build();

    for (int i = 0; i < _globalDescriptionSets.size(); i++) 
    {
        auto bufferInfo = _globalUniformBuffers[i]->descriptionInfo();
        DescriptorWriter(*_globalSetLayout, *_globalPool).writeBuffer(0, &bufferInfo).build(_globalDescriptionSets[i]);
    }

    loadGameObjects();
    _simpleRenderSystem.init(_renderer.getSwapChainRenderPass(), _globalSetLayout->getDescriptorSetLayout());
    _viewerObject.transform.translation.z = 2.5f;
}

VulkanDemo::~VulkanDemo()
{
    delete _globalSetLayout;
    delete _globalPool;
}

void VulkanDemo::run(TimeStep timestep)
{
    //_camera.setViewDirection(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.0f, -0.1f));
    _camerController.moveInPlaneXZ(timestep, _viewerObject);
    _camera.setViewYXZ(_viewerObject.transform.translation, _viewerObject.transform.rotation);

    float apsect = _renderer.getAspectRatio();
    _camera.setPerspectiveProjection(glm::radians(50.f), apsect, 0.1f, 100.f);

    if (auto commandBuffer = _renderer.beginFrame())
    {
        int frameIndex = _renderer.getFrameIndex();
        FrameInfo frameInfo { frameIndex, timestep, commandBuffer, _camera, _globalDescriptionSets[frameIndex]};

        //Update
        _uniBufferObj.projectionView = _camera.getProjection() * _camera.getView();
        _uniBufferObj.lightPosition.y = glm::mod(_uniBufferObj.lightPosition.y + 0.001f, glm::two_pi<float>());
        _uniBufferObj.lightPosition.x = glm::mod(_uniBufferObj.lightPosition.x + 0.002f, glm::two_pi<float>());
        //_uniBufferObj.lightPosition.z = glm::mod(_uniBufferObj.lightPosition.z + 0.005f, glm::two_pi<float>());
        _globalUniformBuffers[frameIndex]->writeToBuffer(&_uniBufferObj);

        //Render
        _renderer.beginSwapChainRenderPass(commandBuffer);
        _simpleRenderSystem.renderGameObjects(frameInfo, _gameObjects);
        _renderer.endSwapChainRenderPass(commandBuffer);
        _renderer.endFrame();
    }

    vkDeviceWaitIdle(RenderGlobals::getInstance().getDevice()->getLogicalDevice());
}

void VulkanDemo::loadGameObjects()
{
    GameObject smoothVase = GameObject::createGameObject();
    smoothVase.model = Model::createModelFromFile("Assets/Models/smooth_vase.obj");
    smoothVase.transform.translation = { 0.5f, 0.5f, 0.f };
    smoothVase.transform.scale = glm::vec3(1.5f, 3.f, 1.5f);

    GameObject flatVase = GameObject::createGameObject();
    flatVase.model = Model::createModelFromFile("Assets/Models/flat_vase.obj");
    flatVase.transform.translation = { -0.5f, 0.5f, 0.f };
    flatVase.transform.scale = glm::vec3(3.f, 1.5f, 3.f);

    GameObject floor = GameObject::createGameObject();
    floor.model = Model::createModelFromFile("Assets/Models/quad.obj");
    floor.transform.translation = { 0.f, 0.5f, 0.f };
    floor.transform.scale = glm::vec3(3.f, 1.f, 3.f);

    _gameObjects.push_back(std::move(smoothVase));
    _gameObjects.push_back(std::move(flatVase));
    _gameObjects.push_back(std::move(floor));
}

}//GUST