#ifndef MATERIAL_HDR
#define MATERIAL_HDR

#include "vulkan/vulkan.h"

namespace Gust 
{
//TODO: Come back to add more material stuff.
struct Material 
{
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
};
}

#endif // !MATERIAL_HDR
