#include "Vulkan.h"

#include <cstdlib>
#include <optional>
#include <limits>
#include <chrono>
#include <functional>
#include <set>
#include <algorithm>
#include <fstream>

#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <tiny_obj_loader.h>

#include "Core/Logger.h"
#include "Core/Global.h"

#include "Core/TimeStep.h"
#include "VkInit.h"

namespace
{

const int MAX_FRAMES_IN_FLIGHT = 2;

struct UniformBufferObject 
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 projection;
};

} //TED

namespace Gust
{

Vulkan::Vulkan(const char* title) : _mipLevels(0), _currentFrame(0), _time(0.f), _flashTime(0.f)
{
    RenderGlobals::getInstance();
    initVulkan(title);
}

void Vulkan::waitDevice()
{
    vkDeviceWaitIdle(RenderGlobals::getInstance().getLogicalDevice());
}


void Vulkan::recreateSwapChain()
{
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(Global::getInstance().getWindow(), &width, &height);

    while (width == 0 || height == 0) 
    {
        glfwGetFramebufferSize(Global::getInstance().getWindow(), &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(RenderGlobals::getInstance().getLogicalDevice());

    swapChainCleanUp();

    createSwapChain();
    createImageView();
    createRenderPass();

    createColourResources();
    createDepthResources();
    createFramebuffer();
}

void Vulkan::drawFrame(TimeStep timestep)
{
    //Begin frame render would be this function.
    updateUniformBuffers(_currentFrame, timestep);

    vkWaitForFences(RenderGlobals::getInstance().getLogicalDevice(), 1, &_inFlightFence[_currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex = -1;
    VkResult result = vkAcquireNextImageKHR(RenderGlobals::getInstance().getLogicalDevice(), _swapChain, UINT64_MAX, _imageAvailableSemaphores[_currentFrame],
                                            VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) 
    {
        recreateSwapChain();
        return;
    }
    GUST_CORE_ASSERT(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR, "Failed to acquire swap chain image.");
    
    vkResetFences(RenderGlobals::getInstance().getLogicalDevice(), 1, &_inFlightFence[_currentFrame]);

    vkResetCommandBuffer(_commandBuffers[_currentFrame], 0);
    recordCommandBuffer(_commandBuffers[_currentFrame], imageIndex);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { _imageAvailableSemaphores[_currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_commandBuffers[_currentFrame];

    VkSemaphore signalSemaphore[] = { _renderFinishedSemaphores[_currentFrame] };
    
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphore;

    result = vkQueueSubmit(RenderGlobals::getInstance().getGraphicsQueue(), 1, &submitInfo, _inFlightFence[_currentFrame]);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to submit command buffer.");

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphore;

    VkSwapchainKHR swapchains[] = { _swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    result = vkQueuePresentKHR(RenderGlobals::getInstance().getPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) 
    {
        recreateSwapChain();
        return;
    }

    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to present swap chain image.");

    _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Vulkan::initVulkan(const char* title)
{
    createSwapChain();
    createImageView();
    createRenderPass();

    createColourResources();
    createDepthResources();
    createFramebuffer();

    //TODO: Consider everything below here and how it can be cleaned up.
    createDescriptionSetLayout();
    createGraphicsPipeline();

    createCommandPool();

    createTextureImage();
    createTextureImageView();
    createTextureSampler();

    //createGeometry();
    loadModel();

    createVertexBuffer();
    createIndexBuffer();

    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();

    createCommandBuffer();

    createSyncObjects();
}

void Vulkan::createSwapChain()
{
    _queuefamilyIndices = findQueueFamilies(RenderGlobals::getInstance().getPhysicalDevice(), RenderGlobals::getInstance().getSurface());
    _msaaSamples = RenderGlobals::getInstance().getMSAASamples();

    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(RenderGlobals::getInstance().getPhysicalDevice(), RenderGlobals::getInstance().getSurface());

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extents = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) 
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = RenderGlobals::getInstance().getSurface();
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extents;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndices[] = { _queuefamilyIndices.graphicsFamily.value(), _queuefamilyIndices.presentFamily.value() };

    if (_queuefamilyIndices.graphicsFamily != _queuefamilyIndices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else 
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    VkResult result = vkCreateSwapchainKHR(RenderGlobals::getInstance().getLogicalDevice(), &createInfo, nullptr, &_swapChain);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create swap chain.");

    vkGetSwapchainImagesKHR(RenderGlobals::getInstance().getLogicalDevice(), _swapChain, &imageCount, nullptr);
    _swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(RenderGlobals::getInstance().getLogicalDevice(), _swapChain, &imageCount, _swapChainImages.data());

    _swapChainImageFormat = surfaceFormat.format;
    _swapChainExtent = extents;
}

void Vulkan::createImageView()
{
    _swapChainImageView.resize(_swapChainImages.size());

    for (uint32_t i = 0; i < _swapChainImages.size(); i++) 
    {
        _swapChainImageView[i] = createImageView(_swapChainImages[i], _swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

void Vulkan::createRenderPass()
{
    VkAttachmentDescription colourAttachment = {};
    colourAttachment.format = _swapChainImageFormat;
    colourAttachment.samples = _msaaSamples;
    colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colourAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = _msaaSamples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colourAttachmentResolve = {};
    colourAttachmentResolve.format = _swapChainImageFormat;
    colourAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colourAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colourAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colourAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colourAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colourAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colourAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colourAttachmentRef = {};
    colourAttachmentRef.attachment = 0;
    colourAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colourAttachmentResolveRef = {};
    colourAttachmentResolveRef.attachment = 2;
    colourAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colourAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments = &colourAttachmentResolveRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 3> attachments = { colourAttachment, depthAttachment, colourAttachmentResolve };
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VkResult result = vkCreateRenderPass(RenderGlobals::getInstance().getLogicalDevice(), &renderPassInfo, nullptr, &_renderPass);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create render pass.");
}

void Vulkan::createColourResources()
{
    VkFormat colourFormat = _swapChainImageFormat;

    createImage(_swapChainExtent.width, _swapChainExtent.height, 1, _msaaSamples, colourFormat, VK_IMAGE_TILING_OPTIMAL, 
                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _colourImage, _colourImageMemory);

    _colourImageView = createImageView(_colourImage, colourFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void Vulkan::createDepthResources()
{
    VkFormat depthFormat = findDepthFormat();

    createImage(_swapChainExtent.width, _swapChainExtent.height, 1, _msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, 
               VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _depthImage, _depthImageMemory);

    _depthImageView = createImageView(_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

void Vulkan::createFramebuffer()
{
    _swapChainFramebuffers.resize(_swapChainImageView.size());

    for (size_t i = 0; i < _swapChainImageView.size(); i++) 
    {
        std::array<VkImageView, 3> attachments =
        {
            _colourImageView,
            _depthImageView,
            _swapChainImageView[i]
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = _renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = _swapChainExtent.width;
        framebufferInfo.height = _swapChainExtent.height;
        framebufferInfo.layers = 1;

        VkResult result = vkCreateFramebuffer(RenderGlobals::getInstance().getLogicalDevice(), &framebufferInfo, nullptr, &_swapChainFramebuffers[i]);
        GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create frame buffer.");
    }
}

void Vulkan::createDescriptionSetLayout()
{
    VkDescriptorSetLayoutBinding uniformBufferLayoutBinding = {};
    uniformBufferLayoutBinding.binding = 0;
    uniformBufferLayoutBinding.descriptorCount = 1;
    uniformBufferLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformBufferLayoutBinding.pImmutableSamplers = nullptr;
    uniformBufferLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> binding = { uniformBufferLayoutBinding, samplerLayoutBinding };
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(binding.size());
    layoutInfo.pBindings = binding.data();

    VkResult result = vkCreateDescriptorSetLayout(RenderGlobals::getInstance().getLogicalDevice(), &layoutInfo, nullptr, &_descriptorSetLayout);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to set up the descriptor set layout.");
}

void Vulkan::createGraphicsPipeline()
{
    auto vertexShaderCode = readFile("Assets/Shaders/simple_shader.vert.spv");
    auto fragShaderCode = readFile("Assets/Shaders/simple_shader.frag.spv");

    VkShaderModule vertexShaderModule = createShaderModule(vertexShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
    vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = vertexShaderModule;
    vertexShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStage[] =
    {
        vertexShaderStageInfo,
        fragShaderStageInfo
    };

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescription = Vertex::getAttributeDescription();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssebly = {};
    inputAssebly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssebly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssebly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    //TODO: check this is what you want for a 2D renderer.
    VkPipelineRasterizationStateCreateInfo rasteriser = {};
    rasteriser.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasteriser.depthClampEnable = VK_FALSE;
    rasteriser.rasterizerDiscardEnable = VK_FALSE;
    rasteriser.polygonMode = VK_POLYGON_MODE_FILL;
    rasteriser.lineWidth = 1.f;
    rasteriser.cullMode = VK_CULL_MODE_NONE;
    rasteriser.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasteriser.depthBiasClamp = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = _msaaSamples;

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colourBlendAttachment = {};
    colourBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | 
                                           VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colourBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colourBlending = {};
    colourBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colourBlending.logicOpEnable = VK_FALSE;
    colourBlending.logicOp = VK_LOGIC_OP_COPY;
    colourBlending.attachmentCount = 1;
    colourBlending.pAttachments = &colourBlendAttachment;
    colourBlending.blendConstants[0] = 0.f;
    colourBlending.blendConstants[1] = 0.f;
    colourBlending.blendConstants[2] = 0.f;
    colourBlending.blendConstants[3] = 0.f;

    std::vector<VkDynamicState> dynamicStates = 
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;

    VkResult result = vkCreatePipelineLayout(RenderGlobals::getInstance().getLogicalDevice(), &pipelineLayoutInfo, nullptr, &_pipelineLayout);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create pipeline.");

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStage;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssebly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasteriser;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colourBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = _pipelineLayout;
    pipelineInfo.renderPass = _renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    result = vkCreateGraphicsPipelines(RenderGlobals::getInstance().getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create graphics pipeline");

    vkDestroyShaderModule(RenderGlobals::getInstance().getLogicalDevice(), fragShaderModule, nullptr);
    vkDestroyShaderModule(RenderGlobals::getInstance().getLogicalDevice(), vertexShaderModule, nullptr);
}

void Vulkan::createCommandPool()
{
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = _queuefamilyIndices.graphicsFamily.value();

    VkResult result = vkCreateCommandPool(RenderGlobals::getInstance().getLogicalDevice(), &poolInfo, nullptr, &_commandPool);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create command pool.");
}

void Vulkan::createTextureImage() 
{
    int texWidth = -1;
    int texHeight = -1;
    int texChannel = -1;
    stbi_uc* pixels = stbi_load("Assets/Textures/room.png", &texWidth, &texHeight, &texChannel, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    _mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
    GUST_CORE_ASSERT(pixels == nullptr, "Failed to load texture image.");

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data = nullptr;
    vkMapMemory(RenderGlobals::getInstance().getLogicalDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<uint32_t>(imageSize));
    vkUnmapMemory(RenderGlobals::getInstance().getLogicalDevice(), stagingBufferMemory);

    stbi_image_free(pixels);

    createImage(texWidth, texHeight, _mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, 
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _textureImage, _textureImageMemory);

    transitionImageLayout(_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, 
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, _mipLevels);
    copyBufferToImage(stagingBuffer, _textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    vkDestroyBuffer(RenderGlobals::getInstance().getLogicalDevice(), stagingBuffer, nullptr);
    vkFreeMemory(RenderGlobals::getInstance().getLogicalDevice(), stagingBufferMemory, nullptr);

    generateMipmaps(_textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, _mipLevels);
}

void Vulkan::createTextureImageView() 
{
    _textureImageView = createImageView(_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, _mipLevels);
}

void Vulkan::createTextureSampler() 
{
    VkPhysicalDeviceProperties properties = {};
    vkGetPhysicalDeviceProperties(RenderGlobals::getInstance().getPhysicalDevice(), &properties);

    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.f;
    samplerInfo.maxLod = static_cast<float>(_mipLevels);
    samplerInfo.mipLodBias = 0.f;

    VkResult result = vkCreateSampler(RenderGlobals::getInstance().getLogicalDevice(), &samplerInfo, nullptr, &_textureSampler);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create texture sampler.");
}

void Vulkan::loadModel() 
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "Assets/Models/room.obj")) 
    {
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

    for (const auto& shape : shapes) 
    {
        for (const auto& index : shape.mesh.indices) 
        {
            Vertex vertex = {};

            vertex.pos = 
            {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord = 
            {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.colour = { 1.0f, 1.0f, 1.0f };

            if (uniqueVertices.count(vertex) == 0) 
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(_vertices.size());
                _vertices.push_back(vertex);
            }

            _indices.push_back(uniqueVertices[vertex]);
        }
    }
}

void Vulkan::createGeometry()
{
    _vertices = 
    {
        { {-1.0f, -1.0f, -0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
        { { 1.0f, -1.0f, -0.5f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
        { { 1.0f,  1.0f, -0.5f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
        { {-1.0f,  1.0f, -0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } }
    };

    _indices = { 0, 1, 2, 2, 3, 0 };
}

void Vulkan::createVertexBuffer() 
{
    VkDeviceSize bufferSize = sizeof(_vertices[0]) * _vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                 stagingBuffer, stagingBufferMemory);

    void* data = nullptr;
    vkMapMemory(RenderGlobals::getInstance().getLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, _vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(RenderGlobals::getInstance().getLogicalDevice(), stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _vertexBuffer, _vertexBufferMemory);
    copyBuffer(stagingBuffer, _vertexBuffer, bufferSize);

    vkDestroyBuffer(RenderGlobals::getInstance().getLogicalDevice(), stagingBuffer, nullptr);
    vkFreeMemory(RenderGlobals::getInstance().getLogicalDevice(), stagingBufferMemory, nullptr);
}

void Vulkan::createIndexBuffer()
{
    VkDeviceSize bufferSize = sizeof(_indices[0]) * _indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);

    void* data = nullptr;
    vkMapMemory(RenderGlobals::getInstance().getLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, _indices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(RenderGlobals::getInstance().getLogicalDevice(), stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _indexBuffer, _indexBufferMemory);
    copyBuffer(stagingBuffer, _indexBuffer, bufferSize);

    vkDestroyBuffer(RenderGlobals::getInstance().getLogicalDevice(), stagingBuffer, nullptr);
    vkFreeMemory(RenderGlobals::getInstance().getLogicalDevice(), stagingBufferMemory, nullptr);
}

void Vulkan::createUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    _uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    _uniformBufferMemory.resize(MAX_FRAMES_IN_FLIGHT);
    _uniformBufferMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                     _uniformBuffers[i], _uniformBufferMemory[i]);
        vkMapMemory(RenderGlobals::getInstance().getLogicalDevice(), _uniformBufferMemory[i], 0, bufferSize, 0, &_uniformBufferMapped[i]);
    }
}

void Vulkan::createDescriptorPool()
{
    std::array<VkDescriptorPoolSize, MAX_FRAMES_IN_FLIGHT> poolSize = {};
    poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
    poolInfo.pPoolSizes = poolSize.data();
    poolInfo.maxSets = static_cast<int32_t>(MAX_FRAMES_IN_FLIGHT);

    VkResult result = vkCreateDescriptorPool(RenderGlobals::getInstance().getLogicalDevice(), &poolInfo, nullptr, &_descriptorPool);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create descriptor pool.");
}

void Vulkan::createDescriptorSets() 
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, _descriptorSetLayout);

    VkDescriptorSetAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = _descriptorPool;
    allocateInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocateInfo.pSetLayouts = layouts.data();

    _descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    VkResult result = vkAllocateDescriptorSets(RenderGlobals::getInstance().getLogicalDevice(), &allocateInfo, _descriptorSets.data());
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to allocate descriptor sets.");

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = _uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = _textureImageView;
        imageInfo.sampler = _textureSampler;

        std::array<VkWriteDescriptorSet, 2> writeDescriptorSets = {};
        writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets[0].dstSet = _descriptorSets[i];
        writeDescriptorSets[0].dstBinding = 0;
        writeDescriptorSets[0].dstArrayElement = 0;
        writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSets[0].descriptorCount = 1;
        writeDescriptorSets[0].pBufferInfo = &bufferInfo;

        writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets[1].dstSet = _descriptorSets[i];
        writeDescriptorSets[1].dstBinding = 1;
        writeDescriptorSets[1].dstArrayElement = 0;
        writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorSets[1].descriptorCount = 1;
        writeDescriptorSets[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(RenderGlobals::getInstance().getLogicalDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
    }
}

void Vulkan::createCommandBuffer()
{
    _commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = _commandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size());

    VkResult result = vkAllocateCommandBuffers(RenderGlobals::getInstance().getLogicalDevice(), &allocateInfo, _commandBuffers.data());
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to allocate command pool.");
}

void Vulkan::createSyncObjects() 
{
    _imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _inFlightFence.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
    {
        VkResult result = vkCreateSemaphore(RenderGlobals::getInstance().getLogicalDevice(), &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]);
        GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create synchronisation objects for a frame");

        result = vkCreateSemaphore(RenderGlobals::getInstance().getLogicalDevice(), &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]);
        GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create synchronisation objects for a renderering");

        result = vkCreateFence(RenderGlobals::getInstance().getLogicalDevice(), &fenceInfo, nullptr, &_inFlightFence[i]);
        GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create synchronisation objects for frames in flight.");
    }
}

void Vulkan::swapChainCleanUp() 
{
    vkDestroyImageView(RenderGlobals::getInstance().getLogicalDevice(), _depthImageView, nullptr);
    vkDestroyImage(RenderGlobals::getInstance().getLogicalDevice(), _depthImage, nullptr);
    vkFreeMemory(RenderGlobals::getInstance().getLogicalDevice(), _depthImageMemory, nullptr);

    vkDestroyImageView(RenderGlobals::getInstance().getLogicalDevice(), _colourImageView, nullptr);
    vkDestroyImage(RenderGlobals::getInstance().getLogicalDevice(), _colourImage, nullptr);
    vkFreeMemory(RenderGlobals::getInstance().getLogicalDevice(), _colourImageMemory, nullptr);

    for (auto framebuffer : _swapChainFramebuffers) 
    {
        vkDestroyFramebuffer(RenderGlobals::getInstance().getLogicalDevice(), framebuffer, nullptr);
    }

    for (auto imageView : _swapChainImageView) 
    {
        vkDestroyImageView(RenderGlobals::getInstance().getLogicalDevice(), imageView, nullptr);
    }

    vkDestroySwapchainKHR(RenderGlobals::getInstance().getLogicalDevice(), _swapChain, nullptr);
}

VkFormat Vulkan::findDepthFormat()
{
    return findSupportedFormat(
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
}

VkFormat Vulkan::findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates) 
    {
        VkFormatProperties props = {};
        vkGetPhysicalDeviceFormatProperties(RenderGlobals::getInstance().getPhysicalDevice(), format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) 
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) 
        {
            return format;
        }

        GUST_CORE_ASSERT(true, "Failed to find supported format.")
    }
}

uint32_t Vulkan::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(RenderGlobals::getInstance().getPhysicalDevice(), &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) 
    {
        if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    GUST_CORE_ASSERT(true, "Failed to find suitable memory type.");
}

VkSurfaceFormatKHR Vulkan::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) 
{
    for (const auto& availableFormat : availableFormats) 
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR Vulkan::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& avaiablePresentModes) 
{
    for (const auto& availablePresentMode : avaiablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Vulkan::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) 
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
    {
        return capabilities.currentExtent;
    }

    int width = 0;
    int height = 0;

    glfwGetFramebufferSize(Global::getInstance().getWindow(), &width, &height);

    VkExtent2D actualExtent =
    {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
}

void Vulkan::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
                          VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(RenderGlobals::getInstance().getLogicalDevice(), &bufferInfo, nullptr, &buffer);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create buffer.");

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(RenderGlobals::getInstance().getLogicalDevice(), buffer, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties);

    result = vkAllocateMemory(RenderGlobals::getInstance().getLogicalDevice(), &allocateInfo, nullptr, &bufferMemory);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to allocate buffer memory.");

    vkBindBufferMemory(RenderGlobals::getInstance().getLogicalDevice(), buffer, bufferMemory, 0);
}

VkImageView Vulkan::createImageView(VkImage image, VkFormat format, VkImageAspectFlagBits aspectFlags, uint32_t mipLevels) 
{
    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;
    createInfo.subresourceRange.aspectMask = aspectFlags;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = mipLevels;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    VkResult result = vkCreateImageView(RenderGlobals::getInstance().getLogicalDevice(), &createInfo, nullptr, &imageView);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create image view");

    return imageView;
}

void Vulkan::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
                         VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                         VkImage& image, VkDeviceMemory& imageMemory) 
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = numSamples;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  

    VkResult result = vkCreateImage(RenderGlobals::getInstance().getLogicalDevice(), &imageInfo, nullptr, &image);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create image.");

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(RenderGlobals::getInstance().getLogicalDevice(), image, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties);

    result = vkAllocateMemory(RenderGlobals::getInstance().getLogicalDevice(), &allocateInfo, nullptr, &imageMemory);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create image memory.");

    vkBindImageMemory(RenderGlobals::getInstance().getLogicalDevice(), image, imageMemory, 0);
}

void Vulkan::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) 
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&  newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else 
    {
        GUST_CORE_ASSERT(true, "Unsupported layout transtional.");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 
        0, 
        0, nullptr,
        0, nullptr,
        1, &barrier);

    endSingleTimeCommand(commandBuffer);
}

void Vulkan::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = 
    {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommand(commandBuffer);
}

void Vulkan::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevel) 
{
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(RenderGlobals::getInstance().getPhysicalDevice(), imageFormat, &formatProperties);
    bool propertyImageFormatResult = (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT);
    GUST_CORE_ASSERT(propertyImageFormatResult != true, "Texture image format does not support linear blitting.");

    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    for (uint32_t i = 1; i < mipLevel; i++) 
    {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);

        VkImageBlit blit = {};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;

        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                      image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                      1, &blit, VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);

        if (mipWidth > 1)
        {
            mipWidth /= 2;
        }
        if (mipHeight > 1) 
        {
            mipHeight /= 2;
        }
    }

    barrier.subresourceRange.baseMipLevel = mipLevel - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);

    endSingleTimeCommand(commandBuffer);
}

VkShaderModule Vulkan::createShaderModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule(RenderGlobals::getInstance().getLogicalDevice(), &createInfo, nullptr, &shaderModule);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create a shader module.");

    return shaderModule;
}

VkCommandBuffer Vulkan::beginSingleTimeCommands() 
{
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandPool = _commandPool;
    allocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(RenderGlobals::getInstance().getLogicalDevice(), &allocateInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Vulkan::endSingleTimeCommand(VkCommandBuffer commandBuffer) 
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(RenderGlobals::getInstance().getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(RenderGlobals::getInstance().getGraphicsQueue());

    vkFreeCommandBuffers(RenderGlobals::getInstance().getLogicalDevice(), _commandPool, 1, &commandBuffer);
}

void Vulkan::copyBuffer(VkBuffer sourceBuffer, VkBuffer destBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, sourceBuffer, destBuffer, 1, &copyRegion);

    endSingleTimeCommand(commandBuffer);
}

void Vulkan::updateUniformBuffers(uint32_t currentImage, TimeStep timestep)
{
    _time += timestep;
    float aspect = 1280.f / 720.f;

    //model = glm::translate(glm::mat4x4(1.f), { 0.f, -2.5f, 0.f }) *
    //                       glm::rotate(glm::mat4x4(1.f), _time * glm::radians(90.f),
    //                       glm::vec3(0, 0, 1));


    //TODO: Move the camera calculations out of here and into a camera class.
    UniformBufferObject uniformBufferObj;
    //uniformBufferObj.view = glm::inverse(glm::translate(glm::mat4x4(1.f), { 0.f, 0.0f, 0.f }) *
    //                                     glm::rotate(glm::mat4x4(1.f), glm::radians(0.f),
    //                                     glm::vec3(0, 0, 1)));
    //uniformBufferObj.projection = glm::ortho(-aspect * 4.5f, aspect * 4.5f, -4.5f, 4.5f);

    //TODO: Design idea, do something like uniformBufferObj.model = Entity.getMeshTransform();
    uniformBufferObj.model = glm::rotate(glm::mat4(1.0f), _time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    uniformBufferObj.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    uniformBufferObj.projection = glm::perspective(glm::radians(45.0f), _swapChainExtent.width / (float)_swapChainExtent.height, 0.1f, 10.0f);
    uniformBufferObj.projection[1][1] *= -1;

    //for (int i = 0; i < _vertices.size(); i++) 
    //{
    //    _vertices[i].pos = _vertices[i].pos * model;
    //}

    memcpy(_uniformBufferMapped[currentImage], &uniformBufferObj, sizeof(uniformBufferObj));
}

void Vulkan::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to begin recording command buffer.");

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _renderPass;
    renderPassInfo.framebuffer = _swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = _swapChainExtent;

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = { { 0.2f, 0.2f, 0.2f, 0.2f } };
    clearValues[1].depthStencil = { 1.f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

    VkViewport viewport = {};
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width = static_cast<float>(_swapChainExtent.width);
    viewport.height = static_cast<float>(_swapChainExtent.height);
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = _swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkBuffer vertexBuffers[] = { _vertexBuffer };
    VkDeviceSize offset[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offset);

    vkCmdBindIndexBuffer(commandBuffer, _indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 
                            0, 1, &_descriptorSets[_currentFrame], 0, nullptr);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(_indices.size()), 1, 0, 0, 0);
    vkCmdEndRenderPass(commandBuffer);

    result = vkEndCommandBuffer(commandBuffer);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to end recording command buffer.");
}

std::vector<char> Vulkan::readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (file.is_open() == false) 
    {
        GUST_CRITICAL("Shader file failed to open {0}", filename);
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> fileBuffer(fileSize);

    file.seekg(0);
    file.read(fileBuffer.data(), fileSize);

    file.close();

    return fileBuffer;
}
}//GUST