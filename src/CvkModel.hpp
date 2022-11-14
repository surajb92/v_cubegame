#pragma once

#include "CvkDevice.hpp"
#include "CvkBuffer.hpp"

// libraries
// Makes sure that in all situations, the GLM functions will expect angles in Radians and not degrees.
#define GLM_FORCE_RADIANS
// Tells GLM to expect depth buffer values from 0 to 1, rather than -1 to 1
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>

namespace cvk {

class CvkModel
{
public:
    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec3 normal{};
        glm::vec2 uv{}; // common shorthand for 2D texture coordinates

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

        bool operator==(const Vertex &other) const {
            return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
        }
    };

    // Temporary helper object to store Vertex and Index information until it can be copied into memory
    struct Builder {
        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices{};

        void loadModel(const std::string &filepath);
    };

    CvkModel(CvkDevice &device, const CvkModel::Builder &builder);
    ~CvkModel();
    
    CvkModel(const CvkModel &) = delete;
    CvkModel &operator=(const CvkModel &) = delete;

    static std::unique_ptr<CvkModel> createModelFromFile(CvkDevice &device, const std::string &filepathh);

    void bind(VkCommandBuffer commandBuffer);
    void draw(VkCommandBuffer commandBuffer);

private:
    void createVertexBuffers(const std::vector<Vertex> &vertices);
    void createIndexBuffers(const std::vector<uint32_t> &indices);

    CvkDevice &cvkDevice;

    std::unique_ptr<CvkBuffer> vertexBuffer;
    uint32_t vertexCount;

    std::unique_ptr<CvkBuffer> indexBuffer;
    uint32_t indexCount;

    bool hasIndexBuffer = false;
};

} // namespace cvk