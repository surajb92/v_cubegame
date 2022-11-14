#pragma once

#include "CvkDevice.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace cvk {

class CvkDescriptorSetLayout {
public:
    class Builder { // Convenient class for building the Descriptor set layouts (blueprint required by pipeline)
    public:
        Builder(CvkDevice &cvkDevice) : cvkDevice{cvkDevice} {}

        Builder &addBinding( // Appends information required by Vulkan to the map of bindings
            uint32_t binding,
            VkDescriptorType descriptorType,
            VkShaderStageFlags stageFlags,
            uint32_t count = 1); // Each binding can have an array of descriptors (same type)
        std::unique_ptr<CvkDescriptorSetLayout> build() const;

    private:
        CvkDevice &cvkDevice;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
    };

    CvkDescriptorSetLayout(
        CvkDevice &cvkDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
    ~CvkDescriptorSetLayout();
    CvkDescriptorSetLayout(const CvkDescriptorSetLayout &) = delete;
    CvkDescriptorSetLayout &operator=(const CvkDescriptorSetLayout &) = delete;

    VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

private:
    CvkDevice &cvkDevice;
    VkDescriptorSetLayout descriptorSetLayout;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

    friend class CvkDescriptorWriter;
};

class CvkDescriptorPool {
public:
    class Builder {
    public:
        Builder(CvkDevice &cvkDevice) : cvkDevice{cvkDevice} {}

        Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
        Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
        Builder &setMaxSets(uint32_t count);
        std::unique_ptr<CvkDescriptorPool> build() const;

    private:
        CvkDevice &cvkDevice;
        std::vector<VkDescriptorPoolSize> poolSizes{};
        uint32_t maxSets = 1000;
        VkDescriptorPoolCreateFlags poolFlags = 0;
    };
    CvkDescriptorPool(
        CvkDevice &cvkDevice,
        uint32_t maxSets,
        VkDescriptorPoolCreateFlags poolFlags,
        const std::vector<VkDescriptorPoolSize> &poolSizes);
    ~CvkDescriptorPool();
    CvkDescriptorPool(const CvkDescriptorPool &) = delete;
    CvkDescriptorPool &operator=(const CvkDescriptorPool &) = delete;

    bool allocateDescriptorSet(
        const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;

    void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

    void resetPool();

private:
    CvkDevice &cvkDevice;
    VkDescriptorPool descriptorPool;

    friend class CvkDescriptorWriter;
};



class CvkDescriptorWriter {
public:
    CvkDescriptorWriter(CvkDescriptorSetLayout &setLayout, CvkDescriptorPool &pool);

    CvkDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
    CvkDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

    bool build(VkDescriptorSet &set);
    void overwrite(VkDescriptorSet &set);

private:
    CvkDescriptorSetLayout &setLayout;
    CvkDescriptorPool &pool;
    std::vector<VkWriteDescriptorSet> writes;
};

} // namespace cvk