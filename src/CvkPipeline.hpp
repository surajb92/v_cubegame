#pragma once

#include "CvkDevice.hpp"

#include <string>
#include <vector>

namespace cvk {

// Need this outside the pipeline class so that the Application has full control and shareability.
struct PipelineConfigInfo {
    PipelineConfigInfo() = default;
    PipelineConfigInfo(const PipelineConfigInfo&) = delete;
    PipelineConfigInfo &operator=(const PipelineConfigInfo&) = delete;

    VkPipelineViewportStateCreateInfo viewportInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    std::vector<VkDynamicState> dynamicStateEnables;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo;
    VkPipelineLayout pipelineLayout = nullptr;
    VkRenderPass renderPass = nullptr;
    uint32_t subpass = 0;
};

class CvkPipeline {
public:
    CvkPipeline(
        CvkDevice& device,
        const std::string&vertFilepath,
        const std::string&fragFilepath,
        const PipelineConfigInfo& configInfo);

    ~CvkPipeline();
    CvkPipeline(const CvkPipeline&) = delete;
    CvkPipeline &operator=(const CvkPipeline&) = delete;

    void bind(VkCommandBuffer commandBuffer);
    static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
private:
    static std::vector<char> readFile(const std::string& filepath);
    void createGraphicsPipeline(
        const std::string& vertFilepath,
        const std::string& fragFilepath,
        const PipelineConfigInfo& configInfo);
    
    void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
    CvkDevice& cvkDevice;
    VkPipeline graphicsPipeline;
    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;
};

} // namespace cvk