#include "CvkRenderer.hpp"

// std
#include <stdexcept>
#include <array>

namespace cvk {

CvkRenderer::CvkRenderer(CvkWindow& window, CvkDevice& device) : cvkWindow{window}, cvkDevice{device} {
    recreateSwapChain();
    createCommandBuffers();   
}
CvkRenderer::~CvkRenderer() {
    freeCommandBuffers();
}

void CvkRenderer::recreateSwapChain() {
    auto extent = cvkWindow.getExtent();
    // While Window is minimized, or being resized, the app will pause and wait.
    while(extent.width == 0 || extent.height == 0) {
        extent = cvkWindow.getExtent();
        glfwWaitEvents(); 
    }
    // Wait until current swapchain is no longer being used before creating new one.
    vkDeviceWaitIdle(cvkDevice.device());

    if (cvkSwapchain == nullptr) {
        cvkSwapchain = std::make_unique<CvkSwapchain>(cvkDevice, extent);
    } else {
        std::shared_ptr<CvkSwapchain> old_SwapChain = std::move(cvkSwapchain);
        cvkSwapchain = std::make_unique<CvkSwapchain>(cvkDevice, extent, old_SwapChain);
        if (!old_SwapChain->compareSwapFormats(*cvkSwapchain.get())) {
            throw std::runtime_error("Swap chain image (or depth) format has changed!");
        }
    }
}

void CvkRenderer::createCommandBuffers() {
    commandBuffers.resize(CvkSwapchain::MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = cvkDevice.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
    if (vkAllocateCommandBuffers(cvkDevice.device(),&allocInfo,commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate Command Buffers!");
    }
}

void CvkRenderer::freeCommandBuffers() {
    vkFreeCommandBuffers(
        cvkDevice.device(),
        cvkDevice.getCommandPool(),
        static_cast<uint32_t>(commandBuffers.size()),
        commandBuffers.data());
    commandBuffers.clear();
}

VkCommandBuffer CvkRenderer::beginFrame() {
    assert(!isFrameStarted && "Can't call beginFrame while already in progress");
    auto result = cvkSwapchain->acquireNextImage(&currentImageIndex);

    // Error that occurs when the surface has changed/resized and is no longer compatible with the swapchain.
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return nullptr;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire Swap Chain image!");
    }

    isFrameStarted = true;
    auto commandBuffer = getCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording Command Buffer!");
    }
    return commandBuffer;
}
void CvkRenderer::endFrame() {
    assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
    auto commandBuffer = getCurrentCommandBuffer();
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record Command Buffer!");
    }
    auto result = cvkSwapchain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || cvkWindow.wasWindowResized()) {
        cvkWindow.resetWindowResizedFlag();
        recreateSwapChain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to present Swap Chain image!");
    }
    isFrameStarted = false;
    currentFrameIndex = (currentFrameIndex + 1) % CvkSwapchain::MAX_FRAMES_IN_FLIGHT;
}

void CvkRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
    assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
    assert(
        commandBuffer == getCurrentCommandBuffer() &&
        "Can't begin render pass on a command buffer from a different frame");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = cvkSwapchain->getRenderPass();
    renderPassInfo.framebuffer = cvkSwapchain->getFrameBuffer(currentImageIndex);

    renderPassInfo.renderArea.offset = {0,0};
    renderPassInfo.renderArea.extent = cvkSwapchain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,VK_SUBPASS_CONTENTS_INLINE);
    
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.width = static_cast<float>(cvkSwapchain->getSwapChainExtent().width);
    viewport.y = 0.0f;
    viewport.height = static_cast<float>(cvkSwapchain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{{0, 0}, cvkSwapchain->getSwapChainExtent()};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}
void CvkRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
    assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
    assert(
        commandBuffer == getCurrentCommandBuffer() &&
        "Can't end render pass on a command buffer from a different frame");
    vkCmdEndRenderPass(commandBuffer);
}

} // namespace cvk