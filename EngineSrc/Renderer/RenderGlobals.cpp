#include "RenderGlobals.h"

#include <set>

#include "Core/Global.h"
#include "Core/Logger.h"

const static bool ENABLE_VALIDATION_LAYER = true;

const static std::vector<const char*> validationLayers =
{
    "VK_LAYER_KHRONOS_validation"
};

static const std::vector<const char*> deviceExtensions =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

namespace Gust 
{

RenderGlobals& RenderGlobals::getInstance()
{
    static RenderGlobals instance;
    return instance;
}

VkSurfaceKHR RenderGlobals::getSurface() const
{
    return _surface;
}

VkPhysicalDevice RenderGlobals::getPhysicalDevice() const
{
    return _physicalDevice;
}

VkDevice RenderGlobals::getLogicalDevice() const 
{
    return _logicalDevice;
}

VkQueue RenderGlobals::getGraphicsQueue() const 
{
    return _graphicsQueue;
}

VkQueue RenderGlobals::getPresentQueue() const 
{
    return _presentQueue;
}

QueueFamilyIndices RenderGlobals::getFamilyIndices() const
{
    return _queuefamilyIndices;
}

VkSampleCountFlagBits RenderGlobals::getMSAASamples() const 
{
    return _msaaSamples;
}

RenderGlobals::RenderGlobals() 
{
    createInstance();

    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
}


void RenderGlobals::createInstance()
{
    if (checkValidateLayerSupport() == false && ENABLE_VALIDATION_LAYER)
    {
        GUST_CRITICAL("Validation layers requested but none available.");
    }

    //Optional setup but good for optimiation
    VkApplicationInfo appInfo = {};
    //These sType's define the type of struct we are setting up.
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Gust Engine";
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

void RenderGlobals::setupDebugMessenger()
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

void RenderGlobals::createSurface()
{
    VkResult result = glfwCreateWindowSurface(_instance, Global::getInstance().getWindow(), nullptr, &_surface);

    GUST_CORE_ASSERT(result != VK_SUCCESS, "Vulkan failed to create window surface.")
}

void RenderGlobals::pickPhysicalDevice()
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

void RenderGlobals::createLogicalDevice()
{
    //_queuefamilyIndices = findQueueFamilies(_physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    //TODO: Another set to try and get rid of.
    std::set<uint32_t> uniqueQueueFamilies = { _queuefamilyIndices.graphicsFamily.value(), _queuefamilyIndices.presentFamily.value() };
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

    vkGetDeviceQueue(_logicalDevice, _queuefamilyIndices.graphicsFamily.value(), 0, &_graphicsQueue);
    vkGetDeviceQueue(_logicalDevice, _queuefamilyIndices.presentFamily.value(), 0, &_presentQueue);
}

bool RenderGlobals::checkValidateLayerSupport()
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

std::vector<const char*> RenderGlobals::getRequiredExtensions()
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

void RenderGlobals::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
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

bool RenderGlobals::isDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamily fam;
    _queuefamilyIndices = fam.findQueueFamilies(device, _surface);

    bool extensionSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return _queuefamilyIndices.isComplete() && extensionSupported && swapChainAdequate;
}

bool RenderGlobals::checkDeviceExtensionSupport(VkPhysicalDevice device)
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

VkSampleCountFlagBits RenderGlobals::getMaxUsableSampleCount()
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


VkResult RenderGlobals::createDebugUtilMessagerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* createInfo,
                                                   const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* debugMessenger)
{
    auto function = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (function != nullptr)
    {
        return function(instance, createInfo, allocator, debugMessenger);
    }

    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

VKAPI_ATTR VkBool32 VKAPI_CALL RenderGlobals::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
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

}//GUST