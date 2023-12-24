#include "Descriptors.h"

#include "Core/Logger.h"

namespace 
{
    VkDescriptorSetLayout DESCRIPTOR_SET_LAYOUT;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> BINDINGS{};
    VkDescriptorPool DESCRIPTOR_POOL;
}

namespace Gust 
{

DescriptorPool::Builder::Builder() : _maxSets(1000), _poolFlags(0)
{
}

DescriptorPool::Builder& DescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType, uint32_t count) 
{
    _poolSizes.push_back({descriptorType, count});
    return *this;
}

DescriptorPool::Builder& DescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags) 
{
    _poolFlags = flags;
    return *this;
}

DescriptorPool::Builder& DescriptorPool::Builder::setMaxSets(uint32_t count) 
{
    _maxSets = count;
    return *this;
}

DescriptorPool* DescriptorPool::Builder::build() const 
{
    return new DescriptorPool(_maxSets, _poolFlags, _poolSizes);
}

DescriptorPool::DescriptorPool(uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags,
                               const std::vector<VkDescriptorPoolSize>& poolSizes)
{
    VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
    descriptorPoolInfo.maxSets = maxSets;
    descriptorPoolInfo.flags = poolFlags;

    VkResult result = vkCreateDescriptorPool(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), 
                                             &descriptorPoolInfo, nullptr, &DESCRIPTOR_POOL);
    GUST_CORE_ASSERT(result != VK_SUCCESS, "Cannot create descriptor pool.");
}

DescriptorPool::~DescriptorPool() 
{
    vkDestroyDescriptorPool(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), DESCRIPTOR_POOL, nullptr);
}

bool DescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const
{
    VkDescriptorSetAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = DESCRIPTOR_POOL;
    allocateInfo.pSetLayouts = &descriptorSetLayout;
    allocateInfo.descriptorSetCount = 1;

    //This is limited because we are only creating 1 descriptor pool. Maybe find away to allocate more as things scale.
    VkResult result = vkAllocateDescriptorSets(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), &allocateInfo, &descriptor);
    return result == VK_SUCCESS;
}

void DescriptorPool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const 
{
    vkFreeDescriptorSets(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), DESCRIPTOR_POOL,
                         static_cast<uint32_t>(descriptors.size()), descriptors.data());
}

void DescriptorPool::resetPool() 
{
    vkResetDescriptorPool(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), DESCRIPTOR_POOL, 0);
}

DescriptorSetLayout::Builder::Builder()
{
}

DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::addBinding(uint32_t binding, VkDescriptorType descriptorType, 
                                                                       VkShaderStageFlags stageFlags, uint32_t count /*= 1*/) 
{
    GUST_CORE_ASSERT(BINDINGS.count(binding) != 0, "You cannot add a binding already in use.");
    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = descriptorType;
    layoutBinding.descriptorCount = count;
    layoutBinding.stageFlags = stageFlags;
    BINDINGS[binding] = layoutBinding;

    return *this;
}

DescriptorSetLayout* DescriptorSetLayout::Builder::build() const 
{
    return new DescriptorSetLayout(BINDINGS);
}

DescriptorSetLayout::DescriptorSetLayout(std::unordered_map<uint32_t,
                                        VkDescriptorSetLayoutBinding> bindings)
{
    BINDINGS = bindings;
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBinding{};

    for (auto &kv : BINDINGS) 
    {
        setLayoutBinding.push_back(kv.second);
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {};
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBinding.size());
    descriptorSetLayoutInfo.pBindings = setLayoutBinding.data();

    VkResult result = vkCreateDescriptorSetLayout(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), &descriptorSetLayoutInfo,
                                                  nullptr, &DESCRIPTOR_SET_LAYOUT);

    GUST_CORE_ASSERT(result != VK_SUCCESS, "Cannout create descriptor set layout.");
}

DescriptorSetLayout::~DescriptorSetLayout() 
{
    vkDestroyDescriptorSetLayout(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), DESCRIPTOR_SET_LAYOUT, nullptr);
}

VkDescriptorSetLayout DescriptorSetLayout::getDescriptorSetLayout() const 
{
    return DESCRIPTOR_SET_LAYOUT;
}

DescriptorWriter::DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool) :
     _poolDescriptor(pool), _setLayoutDescriptor(setLayout)
{
}

DescriptorWriter& DescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo) 
{
    GUST_CORE_ASSERT(BINDINGS.count(binding) != 1, "Layout does not contain specified binding.");
    auto& bindingDescription = BINDINGS[binding];

    GUST_CORE_ASSERT(bindingDescription.descriptorCount != 1, "Binding single descriptor info but binding not expecting multiple.");

    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pBufferInfo = bufferInfo;
    write.descriptorCount = 1;

    _writes.push_back(write);

    return *this;
}

DescriptorWriter& DescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo) 
{
    GUST_CORE_ASSERT(BINDINGS.count(binding) != 1, "Layout does not contain specified binding.");
    auto& bindingDescription = BINDINGS[binding];

    GUST_CORE_ASSERT(bindingDescription.descriptorCount != 1, "Binding single descriptor info but binding not expecting multiple.");

    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pImageInfo = imageInfo;
    write.descriptorCount = 1;

    _writes.push_back(write);

    return *this;
}

bool DescriptorWriter::build(VkDescriptorSet& set) 
{
    bool success = _poolDescriptor.allocateDescriptor(_setLayoutDescriptor.getDescriptorSetLayout(), set);

    if (success == false) 
    {
        return false;
    }
    overwrite(set);

    return true;
}

void DescriptorWriter::overwrite(VkDescriptorSet& set) 
{
    for (auto &write : _writes) 
    {
        write.dstSet = set;
    }

    vkUpdateDescriptorSets(RenderGlobals::getInstance().getDevice()->getLogicalDevice(), 
                           static_cast<uint32_t>(_writes.size()), _writes.data(), 0, nullptr);
}


}//Gust