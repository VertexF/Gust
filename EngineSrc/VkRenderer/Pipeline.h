#ifndef PIPELINE_HDR
#define PIPELINE_HDR

#include <vector>

#include "RenderGlobals.h"

namespace Gust
{
struct PipelineConfigInfo 
{
    PipelineConfigInfo() 
    {
        inputAssembly = {};
        rasterisationInfo = {};
        multisampleInfo = {};
        colourBlendAttachment = {};
        colourBlendInfo = {};
        depthStencil = {};
        dynamicStateInfo = {};
    }

    PipelineConfigInfo(const PipelineConfigInfo&) = delete;
    PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    VkPipelineRasterizationStateCreateInfo rasterisationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineColorBlendAttachmentState colourBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colourBlendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencil;
    std::vector<VkDynamicState> dynamicStateEnables;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo;
    VkPipelineLayout pipelineLayout = nullptr;
    VkRenderPass renderPass = nullptr;
    uint32_t subpass = 0;
};

class Pipeline
{
public:
    Pipeline(const char* vertexFilePath, const char* fragFilePath, const PipelineConfigInfo& config);
    ~Pipeline();

    Pipeline(const Pipeline& rhs) = delete;
    Pipeline& operator=(const Pipeline& rhs) = delete;

    void bind(VkCommandBuffer commandBuffer);

    static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
private:
    static std::vector<char> readFile(const char* filepath);
    void createGraphicsPipeline(const char* vertexFilePath, const char* fragFilePath, const PipelineConfigInfo& config);

    void createShaderModule(const std::vector<char>& code, VkShaderModule *shaderModule);

    VkPipeline _graphicPipeline;
    VkShaderModule _vertShaderModule;
    VkShaderModule _fragShaderModule;
};

}//GUST

#endif // !PIPELINE_HDR