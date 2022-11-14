#pragma once
#include "CvkCamera.hpp"

// libraries
#include <vulkan/vulkan.h>

namespace cvk {

/*
Frame Info makes it easier for render systems etc. to access external information.
*/

struct FrameInfo {
    int frameIndex;
    float frameTime;
    VkCommandBuffer commandBuffer;
    CvkCamera &camera;
    VkDescriptorSet globalDescriptorSet;
};

}; // namespace cvk