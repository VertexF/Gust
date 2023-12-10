#ifndef VULKAN_DEMO
#define VULKAN_DEMO

#include <vector>

#include "Core/Global.h"
#include "Core/TimeStep.h"

#include "Pipeline.h"
#include "RenderGlobals.h"
#include "Renderer.h"
#include "GameObject.h"
#include "Camera.h"
#include "RenderSystem.h"
#include "CameraController.h"
#include "Buffer.h"
#include "Descriptors.h"

namespace Gust 
{
struct GlobalUBO
{
    glm::mat4 projectionView{ 1.f };
    glm::vec4 ambientLightColour{ 1.f, 1.f, 1.f, 0.03f };
    glm::vec3 lightPosition{ 0.f };
    alignas(16) glm::vec4 lightColour{ 1.f };
};


class VulkanDemo
{
public:
    VulkanDemo();
    ~VulkanDemo();

    VulkanDemo(const VulkanDemo& rhs) = delete;
    VulkanDemo& operator=(const VulkanDemo& rhs) = delete;

    void run(TimeStep timestep);

private:
    void loadGameObjects();

    Renderer _renderer;

    Camera _camera;
    CameraController _camerController;
    RenderSystem _simpleRenderSystem;

    GameObject _viewerObject;
    std::vector<Buffer*> _globalUniformBuffers;

    DescriptorPool* _globalPool;
    DescriptorSetLayout* _globalSetLayout;
    std::array<VkDescriptorSet, SwapChain::MAX_FRAMES_IN_FLIGHT> _globalDescriptionSets;
    std::vector<GameObject> _gameObjects;

    GlobalUBO _uniBufferObj;
};
}//GUST

#endif // !VULKAN_DEMO
