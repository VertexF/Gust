#ifndef FRAME_INFO_HDR
#define FRAME_INFO_HDR

#include "vulkan/vulkan.h"

#include "Core/TimeStep.h"
#include "Camera.h"

namespace Gust
{

struct FrameInfo 
{
    int frameIndex;
    TimeStep frameTime;

    VkCommandBuffer commandBuffer;
    Camera& camera;
    VkDescriptorSet globalDescriptorSet;
};

}

#endif // !FRAME_INFO_HDR
