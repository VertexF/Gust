#ifndef PIPELINE_BUILDER
#define PIPELINE_BUILDER

#include <vector>

#include "vulkan/vulkan.h"

namespace Gust 
{

struct PipelineBuilder 
{
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineRasterizationStateCreateInfo rasteriser;
    VkPipelineDepthStencilStateCreateInfo depthStencil;
    VkPipelineColorBlendAttachmentState colourBlendAttachment;
    VkPipelineMultisampleStateCreateInfo multisampling;
    VkPipelineDynamicStateCreateInfo dynamicState;
    VkPipelineLayout pipelineLayout;

    VkPipeline buildPipeline(VkDevice logicalDevice, VkRenderPass renderPass);
};

}//GUST

#endif // !PIPELINE_BUILDER
