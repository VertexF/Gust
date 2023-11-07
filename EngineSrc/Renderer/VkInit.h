#ifndef VK_INIT_HDR
#define VK_INIT_HDR

#include <vulkan/vulkan.h>

namespace Gust 
{
    VkCommandPoolCreateInfo commandPoolInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags);
    VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool pool, uint32_t count, VkCommandBufferLevel level);
    VkCommandBufferBeginInfo commandBufferBeginInfo(VkCommandBufferUsageFlags flags);
    VkFramebufferCreateInfo framebufferCreateInfo(VkRenderPass renderPass, VkExtent2D extent);
    VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags);
    VkSemaphoreCreateInfo semaphoreCreateInfo(VkSemaphoreCreateFlags flags);
    VkSubmitInfo submitInfo(VkCommandBuffer *cmd);
    VkPresentInfoKHR presentInfo();
    VkRenderPassBeginInfo renderpassBeginInfo(VkRenderPass renderPass, VkExtent2D windowExtent, VkFramebuffer framebuffer);

    VkPipelineShaderStageCreateInfo pipelineShaderStageInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule);
    VkPipelineVertexInputStateCreateInfo vertexInputStateInfo();
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo(VkPrimitiveTopology topology);
    VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo();
    VkPipelineRasterizationStateCreateInfo rasterisationStateInfo(VkPolygonMode polygonMode);
    VkPipelineMultisampleStateCreateInfo multisamplingStateInfo(VkSampleCountFlagBits sampleFlagBit);
    VkPipelineDynamicStateCreateInfo dynamicStateInfo();
    VkPipelineColorBlendAttachmentState colourBlendAttachmentState();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo();
}

#endif // !VK_INIT_HDR
