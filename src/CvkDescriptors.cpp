#include "CvkDescriptors.hpp"

// std
#include <cassert>
#include <stdexcept>

namespace cvk {

// *************** Descriptor Set Layout Builder *********************

// Make sure a binding at the specified index hasn't already been added, then configures the Descriptor Layout.
CvkDescriptorSetLayout::Builder &CvkDescriptorSetLayout::Builder::addBinding(
uint32_t binding,
VkDescriptorType descriptorType,
VkShaderStageFlags stageFlags,
uint32_t count) {
    assert(bindings.count(binding) == 0 && "Binding already in use");
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = descriptorType;
    layoutBinding.descriptorCount = count;
    layoutBinding.stageFlags = stageFlags;
    bindings[binding] = layoutBinding;
    return *this;
}

std::unique_ptr<CvkDescriptorSetLayout> CvkDescriptorSetLayout::Builder::build() const {
    return std::make_unique<CvkDescriptorSetLayout>(cvkDevice, bindings);
}

// *************** Descriptor Set Layout *********************

CvkDescriptorSetLayout::CvkDescriptorSetLayout(
CvkDevice &cvkDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
: cvkDevice{cvkDevice}, bindings{bindings} {
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
    for (auto kv : bindings) {
        setLayoutBindings.push_back(kv.second);
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
    descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

    if (vkCreateDescriptorSetLayout(
        cvkDevice.device(),
        &descriptorSetLayoutInfo,
        nullptr,
        &descriptorSetLayout) != VK_SUCCESS) {

        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

CvkDescriptorSetLayout::~CvkDescriptorSetLayout() {
    vkDestroyDescriptorSetLayout(cvkDevice.device(), descriptorSetLayout, nullptr);
}

// *************** Descriptor Pool Builder *********************

CvkDescriptorPool::Builder &CvkDescriptorPool::Builder::addPoolSize(
VkDescriptorType descriptorType, uint32_t count) {
    poolSizes.push_back({descriptorType, count});
    return *this;
}

CvkDescriptorPool::Builder &CvkDescriptorPool::Builder::setPoolFlags(
VkDescriptorPoolCreateFlags flags) {
    poolFlags = flags;
    return *this;
}
CvkDescriptorPool::Builder &CvkDescriptorPool::Builder::setMaxSets(uint32_t count) {
    maxSets = count;
    return *this;
}

std::unique_ptr<CvkDescriptorPool> CvkDescriptorPool::Builder::build() const {
    return std::make_unique<CvkDescriptorPool>(cvkDevice, maxSets, poolFlags, poolSizes);
}

// *************** Descriptor Pool *********************

CvkDescriptorPool::CvkDescriptorPool(
CvkDevice &cvkDevice,
uint32_t maxSets,
VkDescriptorPoolCreateFlags poolFlags,
const std::vector<VkDescriptorPoolSize> &poolSizes)
: cvkDevice{cvkDevice} {
    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
    descriptorPoolInfo.maxSets = maxSets;
    descriptorPoolInfo.flags = poolFlags;

    if (vkCreateDescriptorPool(cvkDevice.device(), &descriptorPoolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

CvkDescriptorPool::~CvkDescriptorPool() {
    vkDestroyDescriptorPool(cvkDevice.device(), descriptorPool, nullptr);
}

bool CvkDescriptorPool::allocateDescriptorSet(
const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    allocInfo.descriptorSetCount = 1;

    //    TODO: Might want to create a "DescriptorPoolManager" class that handles this case, and builds 
    //    TODO: a new pool whenever an old pool fills up.
    if (vkAllocateDescriptorSets(cvkDevice.device(), &allocInfo, &descriptor) != VK_SUCCESS) {
        return false;
    }
    return true;
}

void CvkDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const {
    vkFreeDescriptorSets(
        cvkDevice.device(),
        descriptorPool,
        static_cast<uint32_t>(descriptors.size()),
        descriptors.data());
}

void CvkDescriptorPool::resetPool() {
    vkResetDescriptorPool(cvkDevice.device(), descriptorPool, 0);
}

// *************** Descriptor Writer *********************

CvkDescriptorWriter::CvkDescriptorWriter(CvkDescriptorSetLayout &setLayout, CvkDescriptorPool &pool)
: setLayout{setLayout}, pool{pool} {}

CvkDescriptorWriter &CvkDescriptorWriter::writeBuffer(
uint32_t binding, VkDescriptorBufferInfo *bufferInfo) {
    assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

    auto &bindingDescription = setLayout.bindings[binding];

    // TODO : Expand this to handle Descriptor Arrays
    assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pBufferInfo = bufferInfo;
    write.descriptorCount = 1;

    writes.push_back(write);
    return *this;
}

CvkDescriptorWriter &CvkDescriptorWriter::writeImage(
uint32_t binding, VkDescriptorImageInfo *imageInfo) {
    assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

    auto &bindingDescription = setLayout.bindings[binding];

    assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pImageInfo = imageInfo;
    write.descriptorCount = 1;

    writes.push_back(write);
    return *this;
}

bool CvkDescriptorWriter::build(VkDescriptorSet &set) {
    bool success = pool.allocateDescriptorSet(setLayout.getDescriptorSetLayout(), set);
    if (!success) {
        return false;
    }
    overwrite(set);
    return true;
}

void CvkDescriptorWriter::overwrite(VkDescriptorSet &set) {
    for (auto &write : writes) {
        write.dstSet = set;
    }
    vkUpdateDescriptorSets(pool.cvkDevice.device(), writes.size(), writes.data(), 0, nullptr);
}

}  // namespace cvk
