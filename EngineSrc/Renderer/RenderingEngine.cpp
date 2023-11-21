#include "RenderingEngine.h"

#include <set>
#include <algorithm>
#include <fstream>

#include "glm/gtx/transform.hpp"
#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

#include "Core/Global.h"
#include "VkInit.h"
#include "PipelineBuilder.h"
#include "Entity/StaticEntity.h"

namespace
{
const int MAX_FRAMES_IN_FLIGHT = 2;
const bool ENABLE_VALIDATION_LAYER = true;

const std::vector<const char*> validationLayers =
{
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VkResult createDebugUtilMessagerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* createInfo,
    const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* debugMessenger)
{
    auto function = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (function != nullptr)
    {
        return function(instance, createInfo, allocator, debugMessenger);
    }

    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* allocator)
{
    auto function = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (function != nullptr)
    {
        return function(instance, debugMessenger, allocator);
    }
}

struct MeshPushConstants 
{
    glm::vec4 data;
    glm::mat4 modelViewProjection;
};

} //TED

namespace Gust
{

Renderer::Renderer() : _currentFrame(0), _flashTime(0.f)
{
}

void Renderer::drawFrame(TimeStep timestep, int shaderSwitch /*= 0*/)
{
    _flashTime += timestep;

    vkWaitForFences(_logicalDevice, 1, &_inFlightFence[_currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(_logicalDevice, 1, &_inFlightFence[_currentFrame]);

    vkResetCommandBuffer(_commandBuffers[_currentFrame], 0);

    uint32_t imageIndex = -1;
    VkResult result = vkAcquireNextImageKHR(_logicalDevice, _swapChain, UINT64_MAX, _imageAvailableSemaphores[_currentFrame],
        VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain();
        return;
    }
    GUST_CORE_ASSERT(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR, "Failed to acquire swap chain image.");


    VkCommandBufferBeginInfo beginInfo = commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    result = vkBeginCommandBuffer(_commandBuffers[_currentFrame], &beginInfo);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to begin recording command buffer.");


    std::array<VkClearValue, 2> clearValues = {};
    float flash = std::abs(std::sin(_flashTime));
    clearValues[0].color = { { 0.f, 0.f, flash, 0.f } };
    clearValues[1].depthStencil = { 1.f, 0 };

    VkRenderPassBeginInfo renderPassInfo = renderpassBeginInfo(_renderPass, _swapChainExtent, _swapChainFramebuffers[imageIndex]);

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(_commandBuffers[_currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    //if (shaderSwitch == 0)
    //{
    //    vkCmdBindPipeline(_commandBuffers[_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, _colourPipeline);
    //}
    //else 
    //{
    //    vkCmdBindPipeline(_commandBuffers[_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, _redPipeline);
    //}

    vkCmdBindPipeline(_commandBuffers[_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, _colourPipeline);

    VkViewport viewport = {};
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width = static_cast<float>(_swapChainExtent.width);
    viewport.height = static_cast<float>(_swapChainExtent.height);
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;

    vkCmdSetViewport(_commandBuffers[_currentFrame], 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = _swapChainExtent;
    vkCmdSetScissor(_commandBuffers[_currentFrame], 0, 1, &scissor);

    //vkCmdDraw(_commandBuffers[_currentFrame], 3, 1, 0, 0);
    drawWorld(_commandBuffers[_currentFrame], _renderables.data(), _renderables.size());

    vkCmdEndRenderPass(_commandBuffers[_currentFrame]);
    vkEndCommandBuffer(_commandBuffers[_currentFrame]);

    VkSubmitInfo submit = submitInfo(&_commandBuffers[_currentFrame]);

    VkSemaphore waitSemaphores[] = { _imageAvailableSemaphores[_currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = waitSemaphores;
    submit.pWaitDstStageMask = waitStages;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &_commandBuffers[_currentFrame];

    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &_renderFinishedSemaphores[_currentFrame];

    result = vkQueueSubmit(_graphicsQueue, 1, &submit, _inFlightFence[_currentFrame]);
    //GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to submit command buffer.");

    VkPresentInfoKHR present = presentInfo();
    present.pSwapchains = &_swapChain;
    present.swapchainCount = 1;
    present.pWaitSemaphores = &_renderFinishedSemaphores[_currentFrame];
    present.waitSemaphoreCount = 1;
    present.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(_presentQueue, &present);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        recreateSwapChain();
        return;
    }

    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to present swap chain image.");

    _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::initVulkan(const char* title)
{
    createInstance(title);
    setupDebugMessenger();

    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();

    createSwapChain();
    createImageView();
    createRenderPass();

    createColourResources();
    createDepthResources();
    createFramebuffer();

    createCommands();
    createPipeline();
    loadMeshes();
    createScene();
    createSyncStructures();
}

void Renderer::recreateSwapChain()
{
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(Global::getInstance().getWindow(), &width, &height);

    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(Global::getInstance().getWindow(), &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(_logicalDevice);

    swapChainCleanUp();

    createSwapChain();
    createImageView();
    createRenderPass();

    createColourResources();
    createDepthResources();
    createFramebuffer();
}

void Renderer::swapChainCleanUp()
{
    vkDestroyImageView(_logicalDevice, _depthImageView, nullptr);
    vkDestroyImage(_logicalDevice, _depthImage, nullptr);
    vkFreeMemory(_logicalDevice, _depthImageMemory, nullptr);

    vkDestroyImageView(_logicalDevice, _colourImageView, nullptr);
    vkDestroyImage(_logicalDevice, _colourImage, nullptr);
    vkFreeMemory(_logicalDevice, _colourImageMemory, nullptr);

    for (auto framebuffer : _swapChainFramebuffers)
    {
        vkDestroyFramebuffer(_logicalDevice, framebuffer, nullptr);
    }

    for (auto imageView : _swapChainImageView)
    {
        vkDestroyImageView(_logicalDevice, imageView, nullptr);
    }

    vkDestroySwapchainKHR(_logicalDevice, _swapChain, nullptr);
}


void Renderer::createInstance(const char* title)
{
    if (checkValidateLayerSupport() == false && ENABLE_VALIDATION_LAYER)
    {
        GUST_CRITICAL("Validation layers requested but none available.");
    }

    //Optional setup but good for optimiation
    VkApplicationInfo appInfo = {};
    //These sType's define the type of struct we are setting up.
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = title;
    //Setup version
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
    appInfo.pEngineName = "Gust";
    //Setup version
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    std::vector<const char*> extensions = getRequiredExtensions();

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    if (ENABLE_VALIDATION_LAYER)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = dynamic_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo);
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &_instance);

    GUST_CORE_ASSERT(result != VK_SUCCESS, "Vulkan failed to create an instance!");
}

void Renderer::setupDebugMessenger()
{
    if (ENABLE_VALIDATION_LAYER == false)
    {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    VkResult result = createDebugUtilMessagerEXT(_instance, &createInfo, nullptr, &_debugMessenger);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Vulkan failed to create debug messager info.");
}

void Renderer::createSurface()
{
    VkResult result = glfwCreateWindowSurface(_instance, Global::getInstance().getWindow(), nullptr, &_surface);

    GUST_CORE_ASSERT(result != VK_SUCCESS, "Vulkan failed to create window surface.")
}

void Renderer::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

    GUST_CORE_ASSERT(deviceCount == 0, "Failed to find a physical device that supports Vulkan.");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

    for (const auto& device : devices)
    {
        if (isDeviceSuitable(device))
        {
            _physicalDevice = device;
            _msaaSamples = getMaxUsableSampleCount();
            break;
        }
    }

    GUST_CORE_ASSERT(_physicalDevice == VK_NULL_HANDLE, "Failed to find a suitable device.");
}

void Renderer::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    //TODO: Another set to try and get rid of.
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
    float queuePriority = 1.f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (ENABLE_VALIDATION_LAYER)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    VkResult result = vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_logicalDevice);

    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create logical device");
    vkGetDeviceQueue(_logicalDevice, indices.graphicsFamily.value(), 0, &_graphicsQueue);
    vkGetDeviceQueue(_logicalDevice, indices.presentFamily.value(), 0, &_presentQueue);
}

void Renderer::createSwapChain()
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(_physicalDevice);

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
    createInfo.surface = _surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extents;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily)
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

    VkResult result = vkCreateSwapchainKHR(_logicalDevice, &createInfo, nullptr, &_swapChain);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create swap chain.");

    vkGetSwapchainImagesKHR(_logicalDevice, _swapChain, &imageCount, nullptr);
    _swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(_logicalDevice, _swapChain, &imageCount, _swapChainImages.data());

    _swapChainImageFormat = surfaceFormat.format;
    _swapChainExtent = extents;
}

void Renderer::createImageView()
{
    _swapChainImageView.resize(_swapChainImages.size());

    for (uint32_t i = 0; i < _swapChainImages.size(); i++)
    {
        _swapChainImageView[i] = createImageView(_swapChainImages[i], _swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

void Renderer::createRenderPass()
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

    VkResult result = vkCreateRenderPass(_logicalDevice, &renderPassInfo, nullptr, &_renderPass);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create render pass.");
}

void Renderer::createColourResources()
{
    VkFormat colourFormat = _swapChainImageFormat;

    createImage(_swapChainExtent.width, _swapChainExtent.height, 1, _msaaSamples, colourFormat, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _colourImage, _colourImageMemory);

    _colourImageView = createImageView(_colourImage, colourFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void Renderer::createDepthResources()
{
    VkFormat depthFormat = findDepthFormat();

    createImage(_swapChainExtent.width, _swapChainExtent.height, 1, _msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _depthImage, _depthImageMemory);

    _depthImageView = createImageView(_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

void Renderer::createFramebuffer()
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

        VkResult result = vkCreateFramebuffer(_logicalDevice, &framebufferInfo, nullptr, &_swapChainFramebuffers[i]);
        GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create frame buffer.");
    }
}

void Renderer::createCommands() 
{
    _commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(_physicalDevice);

    VkCommandPoolCreateInfo commandPoolCreateInfo = commandPoolInfo(queueFamilyIndices.graphicsFamily.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkResult result = vkCreateCommandPool(_logicalDevice, &commandPoolCreateInfo, nullptr, &_commandPool);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Could not allocate the command buffer pool.");

    VkCommandBufferAllocateInfo commandAllocateInfo = commandBufferAllocateInfo(_commandPool, static_cast<uint32_t>(_commandBuffers.size()), VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    vkAllocateCommandBuffers(_logicalDevice, &commandAllocateInfo, _commandBuffers.data());
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Could not create  the command buffers.");
}

void Renderer::createPipeline() 
{
    bool success = false;
    VkShaderModule colouredTriangleVertexShader;
    success = loadShaderModule("Assets/Shaders/triangle.vert.spv", &colouredTriangleVertexShader);
    GUST_CORE_ASSERT(success != true, "Failed to load the coloured vertex triangle shader");

    VkShaderModule colouredTriangleFragShader;
    success = loadShaderModule("Assets/Shaders/triangle.frag.spv", &colouredTriangleFragShader);
    GUST_CORE_ASSERT(success != true, "Failed to load the coloured fragment triangle shader");

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = pipelineLayoutInfo();

    VkPushConstantRange pushContant = {};
    pushContant.offset = 0;
    pushContant.size = sizeof(MeshPushConstants);
    pushContant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    pipelineLayoutCreateInfo.pPushConstantRanges = &pushContant;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;

    VkResult result = vkCreatePipelineLayout(_logicalDevice, &pipelineLayoutCreateInfo, nullptr, &_pipelineLayout);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Could not create the pipeline layout.");

    PipelineBuilder pipelineBuilder;
    pipelineBuilder.shaderStages.push_back(pipelineShaderStageInfo(VK_SHADER_STAGE_VERTEX_BIT, colouredTriangleVertexShader));
    pipelineBuilder.shaderStages.push_back(pipelineShaderStageInfo(VK_SHADER_STAGE_FRAGMENT_BIT, colouredTriangleFragShader));

    pipelineBuilder.vertexInputInfo = vertexInputStateInfo();
    pipelineBuilder.inputAssembly = inputAssemblyInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pipelineBuilder.viewport.x = 0.f;
    pipelineBuilder.viewport.y = 0.f;
    pipelineBuilder.viewport.width = static_cast<float>(_swapChainExtent.width);
    pipelineBuilder.viewport.height = static_cast<float>(_swapChainExtent.height);
    pipelineBuilder.viewport.minDepth = 0.f;
    pipelineBuilder.viewport.maxDepth = 1.f;

    pipelineBuilder.scissor.offset = { 0, 0 };
    pipelineBuilder.scissor.extent = _swapChainExtent;

    pipelineBuilder.rasteriser = rasterisationStateInfo(VK_POLYGON_MODE_FILL);
    pipelineBuilder.depthStencil = depthStencilStateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS);
    pipelineBuilder.multisampling = multisamplingStateInfo(getMaxUsableSampleCount());
    pipelineBuilder.colourBlendAttachment = colourBlendAttachmentState();
    pipelineBuilder.dynamicState = dynamicStateInfo();

    VertexInputDescription vertexDescription = ModelVertex::getVertexDescription();

    pipelineBuilder.vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
    pipelineBuilder.vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();

    pipelineBuilder.vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
    pipelineBuilder.vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();

    pipelineBuilder.pipelineLayout = _pipelineLayout;
    _colourPipeline = pipelineBuilder.buildPipeline(_logicalDevice, _renderPass);

    createMaterial(_colourPipeline, _pipelineLayout, "DefaultMesh");
    vkDestroyShaderModule(_logicalDevice, colouredTriangleVertexShader, nullptr);
    vkDestroyShaderModule(_logicalDevice, colouredTriangleFragShader, nullptr);

    pipelineBuilder.shaderStages.clear();
}

bool Renderer::loadShaderModule(const char* path, VkShaderModule* outShaderModule)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (file.is_open() == false)
    {
        return false;
    }

    size_t filesize = static_cast<size_t>(file.tellg());
    std::vector<uint32_t> buffer(filesize / sizeof(uint32_t));

    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), filesize);
    file.close();

    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.pNext = nullptr;

    shaderModuleCreateInfo.codeSize = buffer.size() * sizeof(uint32_t);
    shaderModuleCreateInfo.pCode = buffer.data();

    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule(_logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create shader module.");

    *outShaderModule = shaderModule;
    return true;
}

void Renderer::createSyncStructures() 
{
    _imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _inFlightFence.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkResult result = vkCreateSemaphore(_logicalDevice, &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]);
        GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create synchronisation objects for a frame");

        result = vkCreateSemaphore(_logicalDevice, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]);
        GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create synchronisation objects for a renderering");

        result = vkCreateFence(_logicalDevice, &fenceInfo, nullptr, &_inFlightFence[i]);
        GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create synchronisation objects for frames in flight.");
    }
}

void Renderer::loadMeshes()
{
    Mesh triMesh = {};
    triMesh.vertices.resize(3);

    triMesh.vertices[0].position = {  1.f,  1.f, 0.f };
    triMesh.vertices[1].position = { -1.f,  1.f, 0.f };
    triMesh.vertices[2].position = {  0.f, -1.f, 0.f };

    triMesh.vertices[0].colour = { 0.f, 1.f, 0.f };
    triMesh.vertices[1].colour = { 0.f, 1.f, 0.f };
    triMesh.vertices[2].colour = { 0.f, 1.f, 0.f };

    Mesh monkeyMesh = {};
    monkeyMesh.loadModel("C:/Users/Dan/Documents/Development/git/GameEngine/VS/Assets/Models/monkey_smooth.obj");

    uploadMesh(triMesh);
    uploadMesh(monkeyMesh);

    _meshes["monkey"] = monkeyMesh;
    _meshes["triangle"] = triMesh;
}

void Renderer::createScene()
{
    //TODO: This is a very primitive world creation and entity manager all rolled into one function.
    //Move could out later to be more realitic.

    Entity monkey;
    monkey.setMesh(getMesh("monkey"));
    monkey.setMaterial(getMaterial("DefaultMesh"));
    monkey.setTransform(glm::mat4(1.f));

    _renderables.push_back(monkey);

    //for (int x = 20; x <= 20; x++) 
    //{
    //    for (int y = 20; y <= 20; y++) 
    //    {
            //Entity triangle;
            //triangle.setMesh(getMesh("triangle"));
            //triangle.setMaterial(getMaterial("DefaultMesh"));
            //glm::mat4 translation = glm::translate(glm::mat4(1.f), glm::vec3(0, 0, 0));
            //glm::mat4 scale = glm::scale(glm::mat4(1.f), glm::vec3(0.2f, 0.2f, 0.2f));

            //triangle.setTransform(translation * scale);

            //_renderables.push_back(triangle);
    //    }
    //}
}

void Renderer::uploadMesh(Mesh& mesh)
{
    VkDeviceSize bufferSize = sizeof(mesh.vertices[0]) * mesh.vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);

    void* data = nullptr;
    vkMapMemory(_logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, mesh.vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(_logicalDevice, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mesh.vertexBuffer.buffer, mesh.vertexBuffer.bufferMemory);
    copyBuffer(stagingBuffer, mesh.vertexBuffer.buffer, bufferSize);

    vkDestroyBuffer(_logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(_logicalDevice, stagingBufferMemory, nullptr);
}

void Renderer::drawWorld(VkCommandBuffer command, Entity* first, int count)
{
    //TODO: This is a static camera that can't be moved. Move this guy out and pass it in as an 
    // arguement to apply the user to move the camera as they see fit.
    glm::vec3 cameraPosition = { 0.f, -6.f, -10.f };

    glm::mat4 view = glm::translate(glm::mat4(1.f), cameraPosition);
    glm::mat4 projection = glm::perspective(glm::radians(70.f), 1280.f / 720.f, 0.1f, 200.f);
    projection[1][1] *= -1;

    Mesh* lastMesh = nullptr;
    Material* lastMaterial = nullptr;
    for (int i = 0; i < count; i++) 
    {
        Entity& object = first[i];

        //To check if we need to bind a new pipeline line.
        if (object.getMaterial() != lastMaterial) 
        {
            vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, object.getMaterial()->pipeline);
            lastMaterial = object.getMaterial();
        }

        glm::mat4 model = object.getTransform();
        glm::mat4 meshMatrix = projection * view * model;

        MeshPushConstants contants;
        contants.modelViewProjection = meshMatrix;

        vkCmdPushConstants(command, object.getMaterial()->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &contants);

        //To check if we are on a different mesh.
        if (object.getMesh() != lastMesh) 
        {
            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(command, 0, 1, &object.getMesh()->vertexBuffer.buffer, &offset);
            lastMesh = object.getMesh();
        }

        vkCmdDraw(command, object.getMesh()->vertices.size(), 1, 0, 0);
    }
}

bool Renderer::checkValidateLayerSupport()
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> avaiableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, avaiableLayers.data());

    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : avaiableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (layerFound == false)
        {
            return false;
        }
    }

    return true;
}

std::vector<const char*> Renderer::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (ENABLE_VALIDATION_LAYER)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}


void Renderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    createInfo.pfnUserCallback = debugCallback;
}

SwapChainSupportDetails Renderer::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, details.formats.data());
    }

    uint32_t presentMode = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentMode, nullptr);

    if (presentMode != 0)
    {
        details.presentModes.resize(presentMode);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentMode, details.presentModes.data());
    }

    return details;
}

bool Renderer::isDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionSupported && swapChainAdequate;
}

QueueFamilyIndices Renderer::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamilies : queueFamilies)
    {
        if (queueFamilies.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentSupport);

        if (presentSupport)
        {
            indices.presentFamily = i;
        }

        if (indices.isComplete())
        {
            break;
        }

        i++;
    }

    return indices;
}

bool Renderer::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    //TODO: Check if you can get rid of set.
    std::set<std::string> requiredExtension(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions)
    {
        requiredExtension.erase(extension.extensionName);
    }

    return requiredExtension.empty();
}

VkSampleCountFlagBits Renderer::getMaxUsableSampleCount()
{
    VkPhysicalDeviceProperties physicalDeviceProperty;
    vkGetPhysicalDeviceProperties(_physicalDevice, &physicalDeviceProperty);

    VkSampleCountFlags count = physicalDeviceProperty.limits.framebufferColorSampleCounts &
        physicalDeviceProperty.limits.framebufferDepthSampleCounts;

    if (count & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (count & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (count & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (count & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (count & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (count & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
}

VkFormat Renderer::findDepthFormat()
{
    return findSupportedFormat(
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

VkFormat Renderer::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props = {};
        vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &props);

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

uint32_t Renderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    GUST_CORE_ASSERT(true, "Failed to find suitable memory type.");
}

VkSurfaceFormatKHR Renderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
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

VkPresentModeKHR Renderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& avaiablePresentModes)
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

VkExtent2D Renderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
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

void Renderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
    VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(_logicalDevice, &bufferInfo, nullptr, &buffer);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create buffer.");

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(_logicalDevice, buffer, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties);

    result = vkAllocateMemory(_logicalDevice, &allocateInfo, nullptr, &bufferMemory);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to allocate buffer memory.");

    vkBindBufferMemory(_logicalDevice, buffer, bufferMemory, 0);
}

void Renderer::copyBuffer(VkBuffer sourceBuffer, VkBuffer destBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, sourceBuffer, destBuffer, 1, &copyRegion);

    endSingleTimeCommand(commandBuffer);
}

VkImageView Renderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlagBits aspectFlags, uint32_t mipLevels)
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
    VkResult result = vkCreateImageView(_logicalDevice, &createInfo, nullptr, &imageView);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create image view");

    return imageView;
}

void Renderer::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
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

    VkResult result = vkCreateImage(_logicalDevice, &imageInfo, nullptr, &image);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create image.");

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(_logicalDevice, image, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties);

    result = vkAllocateMemory(_logicalDevice, &allocateInfo, nullptr, &imageMemory);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Failed to create image memory.");

    vkBindImageMemory(_logicalDevice, image, imageMemory, 0);
}

VkCommandBuffer Renderer::beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandPool = _commandPool;
    allocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(_logicalDevice, &allocateInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Renderer::endSingleTimeCommand(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_graphicsQueue);

    vkFreeCommandBuffers(_logicalDevice, _commandPool, 1, &commandBuffer);
}

Material* Renderer::createMaterial(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name)
{
    Material material;
    material.pipeline = pipeline;
    material.pipelineLayout = layout;
    _materials[name] = material;

    return &_materials[name];
}

Material* Renderer::getMaterial(const std::string& name)
{
    auto it = _materials.find(name);
    if (it == _materials.end())
    {
        GUST_CORE_ASSERT(true, "Was unable to find the material in the map.");
        return nullptr;
    }

    return &it->second;
}

Mesh* Renderer::getMesh(const std::string& name)
{
    auto it = _meshes.find(name);
    if (it == _meshes.end())
    {
        GUST_CORE_ASSERT(true, "Was unable to find the material in the map.");
        return nullptr;
    }

    return &it->second;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Renderer::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* userData)
{
    switch (messageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        GUST_ERROR("Validation Layer : {0}", callbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        GUST_WARN("Validation Layer : {0}", callbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        GUST_INFO("Validation Layer : {0}", callbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        GUST_TRACE("Validation Layer : {0}", callbackData->pMessage);
        break;
    }

    return false;
}


} //GUST