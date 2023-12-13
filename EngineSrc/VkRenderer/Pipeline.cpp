#include "Pipeline.h"


#include <fstream>

#include "Core/Logger.h"
#include "Model.h"

namespace Gust
{

Pipeline::Pipeline(const char* vertexFilePath, const char* fragFilePath, const PipelineConfigInfo& config)
{
    createGraphicsPipeline(vertexFilePath, fragFilePath, config);
}

Pipeline::~Pipeline() 
{
    vkDestroyShaderModule(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), _vertShaderModule, nullptr);
    vkDestroyShaderModule(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), _fragShaderModule, nullptr);

    vkDestroyPipeline(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), _graphicPipeline, nullptr);
}

void Pipeline::bind(VkCommandBuffer commandBuffer)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicPipeline);
}

void Pipeline::defaultPipelineConfigInfo(PipelineConfigInfo &configInfo)
{
    configInfo.inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    configInfo.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    configInfo.inputAssembly.primitiveRestartEnable = VK_FALSE;

    configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    configInfo.viewportInfo.viewportCount = 1;
    configInfo.viewportInfo.pViewports = nullptr;
    configInfo.viewportInfo.scissorCount = 1;
    configInfo.viewportInfo.pScissors = nullptr;

    configInfo.rasterisationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    configInfo.rasterisationInfo.depthClampEnable = VK_FALSE;
    configInfo.rasterisationInfo.rasterizerDiscardEnable = VK_FALSE;
    configInfo.rasterisationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    configInfo.rasterisationInfo.lineWidth = 1.f;
    configInfo.rasterisationInfo.cullMode = VK_CULL_MODE_NONE;
    configInfo.rasterisationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    configInfo.rasterisationInfo.depthBiasEnable = VK_FALSE;
    configInfo.rasterisationInfo.depthBiasConstantFactor = 0.f;
    configInfo.rasterisationInfo.depthBiasClamp = 0.f;
    configInfo.rasterisationInfo.depthBiasSlopeFactor = 0.f;

    configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
    configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    configInfo.multisampleInfo.minSampleShading = 1.f;
    configInfo.multisampleInfo.pSampleMask = nullptr;
    configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;
    configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;

    configInfo.colourBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT 
                                                    | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    configInfo.colourBlendAttachment.blendEnable = VK_FALSE;
    configInfo.colourBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    configInfo.colourBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    configInfo.colourBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    configInfo.colourBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    configInfo.colourBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    configInfo.colourBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    configInfo.colourBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    configInfo.colourBlendInfo.logicOpEnable = VK_FALSE;
    configInfo.colourBlendInfo.logicOp = VK_LOGIC_OP_COPY;
    configInfo.colourBlendInfo.attachmentCount = 1;
    configInfo.colourBlendInfo.pAttachments = &configInfo.colourBlendAttachment;
    configInfo.colourBlendInfo.blendConstants[0] = 0.f;
    configInfo.colourBlendInfo.blendConstants[1] = 0.f;
    configInfo.colourBlendInfo.blendConstants[2] = 0.f;
    configInfo.colourBlendInfo.blendConstants[3] = 0.f;

    configInfo.depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    configInfo.depthStencil.depthTestEnable = VK_TRUE;
    configInfo.depthStencil.depthWriteEnable = VK_TRUE;
    configInfo.depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    configInfo.depthStencil.depthBoundsTestEnable = VK_FALSE;
    configInfo.depthStencil.minDepthBounds = 0.f;
    configInfo.depthStencil.maxDepthBounds = 1.f;
    configInfo.depthStencil.stencilTestEnable = VK_FALSE;
    configInfo.depthStencil.front = {};
    configInfo.depthStencil.back = {};

    configInfo.dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
    configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
    configInfo.dynamicStateInfo.flags = 0;

    configInfo.bindingDescriptions = Model::Vertex::getBindingDescriptions();
    configInfo.attributeDescriptions = Model::Vertex::getAttributeDescriptions();
}

void Pipeline::enableAlphaBlending(PipelineConfigInfo& configInfo)
{
    configInfo.colourBlendAttachment.blendEnable = VK_TRUE;
    configInfo.colourBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | 
                                                      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    configInfo.colourBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    configInfo.colourBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    configInfo.colourBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    configInfo.colourBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    configInfo.colourBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    configInfo.colourBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
}

std::vector<char> Pipeline::readFile(const char* filepath)
{
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);

    if (file.is_open() == false)
    {
        GUST_CRITICAL("Shader file failed to open {0}", filepath);
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> fileBuffer(fileSize);

    file.seekg(0);
    file.read(fileBuffer.data(), fileSize);

    file.close();

    return fileBuffer;
}

void Pipeline::createGraphicsPipeline(const char* vertexFilePath, const char* fragFilePath, const PipelineConfigInfo& config)
{
    std::vector<char> vertexFileContents = readFile(vertexFilePath);
    std::vector<char> fragFileContents = readFile(fragFilePath);

    createShaderModule(vertexFileContents, &_vertShaderModule);
    createShaderModule(fragFileContents, &_fragShaderModule);

    VkPipelineShaderStageCreateInfo shaderStages[2];
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = _vertShaderModule;
    shaderStages[0].pName = "main";
    shaderStages[0].flags = 0;
    shaderStages[0].pNext = nullptr;
    shaderStages[0].pSpecializationInfo = nullptr;

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = _fragShaderModule;
    shaderStages[1].pName = "main";
    shaderStages[1].flags = 0;
    shaderStages[1].pNext = nullptr;
    shaderStages[1].pSpecializationInfo = nullptr;

    auto& bindingDescription = config.bindingDescriptions; 
    auto& attributeDescription = config.attributeDescriptions;// = Model::Vertex::getAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescription.size());
    vertexInputInfo.pVertexBindingDescriptions = bindingDescription.data();

    VkPipelineViewportStateCreateInfo viewportInfo = {};
    viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.viewportCount = 1;
    viewportInfo.pViewports = nullptr;
    viewportInfo.scissorCount = 1;
    viewportInfo.pScissors = nullptr;

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pViewportState = &viewportInfo;
    pipelineInfo.pInputAssemblyState = &config.inputAssembly;
    pipelineInfo.pRasterizationState = &config.rasterisationInfo;
    pipelineInfo.pMultisampleState = &config.multisampleInfo;
    pipelineInfo.pColorBlendState = &config.colourBlendInfo;
    pipelineInfo.pDepthStencilState = &config.depthStencil;
    pipelineInfo.pDynamicState = &config.dynamicStateInfo;

    pipelineInfo.layout = config.pipelineLayout;
    pipelineInfo.renderPass = config.renderPass;
    pipelineInfo.subpass = config.subpass;

    pipelineInfo.basePipelineIndex = -1;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    VkResult result = vkCreateGraphicsPipelines(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicPipeline);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Could not create graphics pipeline.");
}

void Pipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
{
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = code.size();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkResult result = vkCreateShaderModule(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), &shaderModuleCreateInfo, nullptr, shaderModule);
}

} //GUST
