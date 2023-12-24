#ifndef DESCRIPTOR_HDR
#define DESCRIPTOR_HDR

#include <unordered_map>
#include <vector>

#include "vulkan/vulkan.h"

#include "RenderGlobals.h"

namespace Gust 
{

class DescriptorPool
{
public:
    class Builder 
    {
    public:
        Builder();

        Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
        Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
        Builder& setMaxSets(uint32_t count);

        DescriptorPool* build() const;
    private:
        std::vector<VkDescriptorPoolSize> _poolSizes;
        uint32_t _maxSets;
        VkDescriptorPoolCreateFlags _poolFlags;
    };

    DescriptorPool(uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, 
                  const std::vector<VkDescriptorPoolSize> &poolSizes);
    ~DescriptorPool();

    DescriptorPool(const DescriptorPool& rhs) = delete;
    DescriptorPool& operator==(DescriptorPool& rhs) = delete;

    bool allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;
    void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

    void resetPool();
};

class DescriptorSetLayout
{
public:
    class Builder
    {
    public:
        Builder();
        Builder& addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count = 1);
        DescriptorSetLayout* build() const;

    private:
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
    };

    DescriptorSetLayout(std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
    ~DescriptorSetLayout();

    DescriptorSetLayout(const DescriptorSetLayout& rhs) = delete;
    DescriptorSetLayout& operator==(DescriptorSetLayout& rhs) = delete;

    VkDescriptorSetLayout getDescriptorSetLayout() const;
};

class DescriptorWriter
{
public:
    DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool);

    DescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
    DescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

    bool build(VkDescriptorSet& set);
    void overwrite(VkDescriptorSet &set);

private:
    DescriptorPool& _poolDescriptor;
    DescriptorSetLayout& _setLayoutDescriptor;
    std::vector<VkWriteDescriptorSet> _writes;
};

}//Gust

#endif // !DESCRIPTOR_HDR
