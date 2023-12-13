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
#include "Systems/RenderSystem.h"
#include "Systems/PointLightSystem.h"
#include "CameraController.h"
#include "Buffer.h"
#include "Descriptors.h"
#include "FrameInfo.h"

namespace Gust 
{

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
    PointLightSystem _pointLightSystem;

    GameObject _viewerObject;
    std::vector<Buffer*> _globalUniformBuffers;

    DescriptorPool* _globalPool;
    DescriptorSetLayout* _globalSetLayout;
    std::array<VkDescriptorSet, SwapChain::MAX_FRAMES_IN_FLIGHT> _globalDescriptionSets;
    std::unordered_map<uint32_t, GameObject> _gameObjects;

    GlobalUBO _uniBufferObj;
};
}//GUST

#endif // !VULKAN_DEMO
