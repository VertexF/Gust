#include "Buffer.h"

#include <cstring>

#include "Core/Logger.h"

namespace Gust 
{
Buffer::Buffer(VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags,
                VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment /*= 1*/) 
    : _instanceSize(instanceSize), _instanceCount(instanceCount), 
      _usageFlags(usageFlags), _memoryPropertyFlags(memoryPropertyFlags)
{
    _alignmentSize = getAlignment(_instanceSize, minOffsetAlignment);
    _bufferSize = _alignmentSize * _instanceCount;
    RenderGlobals::getInstance().getDevice()->createBuffer(_bufferSize, _usageFlags, _memoryPropertyFlags, _buffer, _memory);
}

Buffer::~Buffer() 
{
    unmap();
    vkDestroyBuffer(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), _buffer, nullptr);
    vkFreeMemory(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), _memory, nullptr);
}

VkResult Buffer::map(VkDeviceSize size /*= VK_WHOLE_SIZE*/, VkDeviceSize offset /*= 0*/) 
{
    GUST_CORE_ASSERT(_buffer == nullptr && _memory == nullptr, "You cannout map data onto a buffer before it's created.");
    return vkMapMemory(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), _memory, offset, size, 0, &_mapped);
}

void Buffer::unmap() 
{
    if (_mapped) 
    {
        vkUnmapMemory(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), _memory);
        _memory = nullptr;
    }
}

void Buffer::writeToBuffer(void* data, VkDeviceSize size/*= VK_WHOLE_SIZE*/, VkDeviceSize offset /*= 0*/) 
{
    GUST_CORE_ASSERT(_mapped == nullptr, "Cannout copy to an unmapped buffer.");

    if (size == VK_WHOLE_SIZE) 
    {
        memcpy(_mapped, data, _bufferSize);
    }
    else 
    {
        char* memoryOffset = static_cast<char*>(_mapped);
        memoryOffset += offset;
        memcpy(memoryOffset, data, size);
    }
}

VkResult Buffer::flush(VkDeviceSize size /*= VK_WHOLE_SIZE*/, VkDeviceSize offset /*= 0*/) 
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = _memory;
    mappedRange.offset = offset;
    mappedRange.size = size;

    return vkFlushMappedMemoryRanges(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), 1, &mappedRange);
}

VkDescriptorBufferInfo Buffer::descriptionInfo(VkDeviceSize size /*= VK_WHOLE_SIZE*/, VkDeviceSize offset /*= 0*/) 
{
    return VkDescriptorBufferInfo { _buffer, offset, size };
}

VkResult Buffer::invalidate(VkDeviceSize size /*= VK_WHOLE_SIZE*/, VkDeviceSize offset /*= 0*/) 
{
    //This make the buffer visible to the host.
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = _memory;
    mappedRange.offset = offset;
    mappedRange.size = size;

    return vkInvalidateMappedMemoryRanges(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), 1, &mappedRange);
}

void Buffer::writeToIndex(void* data, int index) 
{
    //Copies _instanceSize bytes of data to the mapped buffer at an offset of index *  _alignmentSize.
    writeToBuffer(data, _instanceSize, index * _alignmentSize);
}

VkResult Buffer::flushIndex(int index) 
{
    return flush(_alignmentSize, index * _alignmentSize);
}

VkDescriptorBufferInfo Buffer::descriptorInfoIndex(int index) 
{
    return descriptionInfo(_alignmentSize, index * _alignmentSize);
}

VkResult Buffer::invalidateIndex(int index) 
{
    return invalidate(_alignmentSize, index * _alignmentSize);
}


VkBuffer Buffer::getBuffer() const 
{
    return _buffer;
}

void* Buffer::getMappedMemory() const 
{
    return _mapped;
}

uint32_t Buffer::getInstanceCount() const 
{
    return _instanceCount;
}

VkDeviceSize Buffer::getInstanceSize() const 
{
    return _instanceSize;
}

VkDeviceSize Buffer::getAlignmentSize() const 
{
    return _instanceSize;
}

VkBufferUsageFlags Buffer::getUsageFlags() const 
{
    return _usageFlags;
}

VkMemoryPropertyFlags Buffer::getMemoryPropertyFlags() const 
{
    return _memoryPropertyFlags;
}

VkDeviceSize Buffer::getBufferSize() const 
{
    return _bufferSize;
}

VkDeviceSize Buffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) 
{
    if (minOffsetAlignment > 0) 
    {
        return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
    }
    return instanceSize;
}

}//Gust