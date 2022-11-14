#pragma once

#include "CvkCamera.hpp"
#include "CvkDevice.hpp"
#include "CvkGameObject.hpp"
#include "CvkPipeline.hpp"
#include "CvkFrameInfo.hpp"

// std
#include <memory>

namespace cvk {

class SimpleRenderSystem {
public:

    SimpleRenderSystem(CvkDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
    ~SimpleRenderSystem();

    SimpleRenderSystem(const SimpleRenderSystem &) = delete;
    SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;
    void renderGameObjects(FrameInfo& frameInfo, std::vector<CvkGameObject> &gameObjects);
private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void createPipeline(VkRenderPass renderPass);

    CvkDevice &cvkDevice;

    // Smart pointer simulates a pointer with automatic memory management.
    // So we are no longer responsible for calling new() or delete()
    std::unique_ptr<CvkPipeline> cvkPipeline;
    VkPipelineLayout pipelineLayout;
};

} // namespace cvk