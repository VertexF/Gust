#ifndef FRAME_INFO_HDR
#define FRAME_INFO_HDR

#include <unordered_map>

#include "vulkan/vulkan.h"

#include "Core/TimeStep.h"
#include "Entity/GameObject.h"

#include "Camera.h"
namespace 
{
    const int MAX_LIGHTS = 10;
}//TED

namespace Gust
{

struct PointLight 
{
    glm::vec4 position{};
    glm::vec4 colour{};
};

struct GlobalUBO
{
    glm::mat4 projection{ 1.f };
    glm::mat4 view{ 1.f };
    glm::mat4 inverseView{ 1.f };
    glm::vec4 ambientLightColour{ 0.f, 0.f, 0.f, 0.03f };
    PointLight pointLights[MAX_LIGHTS];
    int numLights;
};

struct FrameInfo 
{
    int frameIndex;
    TimeStep frameTime;

    VkCommandBuffer commandBuffer;
    Camera& camera;
    VkDescriptorSet globalDescriptorSet;

    std::unordered_map<uint32_t, GameObject>& gameObjects;
};

}

#endif // !FRAME_INFO_HDR
