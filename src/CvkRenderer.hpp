#pragma once

#include "CvkDevice.hpp"
#include "CvkSwapchain.hpp"
#include "CvkWindow.hpp"

// std
#include <cassert>
#include <memory>
#include <vector>

namespace cvk {

class CvkRenderer {
public:
    CvkRenderer(CvkWindow &window, CvkDevice &device);
    ~CvkRenderer();

    CvkRenderer(const CvkRenderer &) = delete;
    CvkRenderer &operator=(const CvkRenderer &) = delete;

    VkRenderPass getSwapChainRenderPass() const { return cvkSwapchain->getRenderPass(); }
    float getAspectRatio() const { return cvkSwapchain->extentAspectRatio(); }
    bool isFrameInProgress() const { return isFrameStarted; }

    VkCommandBuffer getCurrentCommandBuffer() const {
        assert(isFrameStarted && "Cannot get Command Buffer when frame is not in progress!");
        return commandBuffers[currentFrameIndex];
    }

    int getFrameIndex() const {
        assert(isFrameStarted && "Cannot get frame index when frame is not in progress!");
        return currentFrameIndex;
    }
    VkCommandBuffer beginFrame();
    void endFrame();
    void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
    void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
    
private:
    void createCommandBuffers();
    void freeCommandBuffers();
    void recreateSwapChain();

    CvkWindow& cvkWindow;
    CvkDevice& cvkDevice;

    // Smart pointer simulates a pointer with automatic memory management.
    // So we are no longer responsible for calling new() or delete()
    std::unique_ptr<CvkSwapchain> cvkSwapchain;
    std::vector<VkCommandBuffer> commandBuffers;

    uint32_t currentImageIndex;
    int currentFrameIndex{0};
    bool isFrameStarted{false};
};

} // namespace cvk