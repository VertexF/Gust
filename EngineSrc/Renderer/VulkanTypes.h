#ifndef VULKAN_TYPES_HDR
#define VULKAN_TYPES_HDR

#include "vulkan/vulkan.h"
#include "vma/vk_mem_alloc.h"

namespace Gust 
{

struct AllocatedBuffer
{
    VkBuffer buffer;
    VkDeviceMemory bufferMemory;
};

struct AllocatedImage 
{
    VkImage image;
    VmaAllocation allocation;
};

}

#endif // !VULKAN_TYPES_HDR
