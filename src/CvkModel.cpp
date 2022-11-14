#include "CvkModel.hpp"
#include "CvkUtils.hpp"

// libraries
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// std
#include <cassert>
#include <cstring>
#include <unordered_map>

namespace std {
template <>
struct hash<cvk::CvkModel::Vertex> {
    size_t operator()(cvk::CvkModel::Vertex const & vertex) const {
        size_t seed = 0;
        cvk::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
        return seed;
    }
};
}

namespace cvk {

CvkModel::CvkModel(CvkDevice &device, const CvkModel::Builder &builder) : cvkDevice{device} {
    createVertexBuffers(builder.vertices);
    createIndexBuffers(builder.indices);
}

CvkModel::~CvkModel() { }

std::unique_ptr<CvkModel> CvkModel::createModelFromFile(CvkDevice &device, const std::string &filepath) {
    Builder builder{};
    builder.loadModel(filepath);
    return std::make_unique<CvkModel>(device, builder);
}

void CvkModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
    vertexCount = static_cast<uint32_t>(vertices.size());
    assert(vertexCount >= 3 && "Vertex count must be at least 3");
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

    /*
    DEVICE_LOCAL_BIT is more efficient GPU memory than HOST_VISIBLE_BIT. But the Host(CPU) memory cannot be mapped to this type of memory. Therefore, we use a temporary (staging) buffer on the GPU to map memory from Host(CPU) and flush data, then we will copy that buffer to the more efficient one (DEVICE_LOCAL_BIT) using vkCopyBuffer.
    After that, we will delete the staging buffer and it's map from the Host(CPU).
    Q. When to use Staging Buffers + Device Local Memory?
    A. When working with Static Data loaded at the start of the App (e.g. 3D Meshes).
    */
    uint32_t vertexSize = sizeof(vertices[0]);
    // 1. Create a temporary (Staging) buffer on Device(GPU)
    CvkBuffer stagingBuffer{
        cvkDevice,
        vertexSize,
        vertexCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, // Only using this buffer as a source for a mem transfer op.
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        // HOST_VISIBLE means that this type of memory can be mapped for Host(CPU) access
        // HOST_COHERENT means, that when the Host(CPU) side updates, it automatically flushes the data to the Device (GPU) memory.
    };
    // 2. Map memory on Host(CPU) to Device(GPU) memory (Staging Buffer's Memory).
    stagingBuffer.map();
    // 3. CPU accessible memory written --> GPU memory also written (since it's mapped above).
    stagingBuffer.writeToBuffer((void *)vertices.data());
    // 4. Create DEVICE_LOCAL Buffer (more optimized) 
    vertexBuffer = std::make_unique<CvkBuffer>(
        cvkDevice,
        vertexSize,
        vertexCount,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        // that last one is so that you can transfer the staging buffer and have this be the destination.
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT // Far more efficient in GPU
    );
    // 5. Copy Staging Buffer to DEVICE_LOCAL Vertex Buffer.
    cvkDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
    // 6. Staging buffer is a local variable, and will be cleaned up here automatically.
}

void CvkModel::createIndexBuffers(const std::vector<uint32_t> &indices) {
    indexCount = static_cast<uint32_t>(indices.size());
    hasIndexBuffer = indexCount > 0;
    if (!hasIndexBuffer) { return; }
    VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;

    // Same Process as Vertex Buffer, refer above.
    uint32_t indexSize = sizeof(indices[0]);
    CvkBuffer stagingBuffer {
        cvkDevice,
        indexSize,
        indexCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void *)indices.data());

    indexBuffer = std::make_unique<CvkBuffer>(
        cvkDevice,
        indexSize,
        indexCount,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    cvkDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
}

void CvkModel::draw(VkCommandBuffer commandBuffer) {
    if (hasIndexBuffer) {
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
    } else {
        vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
    }
}
void CvkModel::bind(VkCommandBuffer commandBuffer) {
    VkBuffer buffers[] = {vertexBuffer->getBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    if (hasIndexBuffer) {
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }
}

std::vector<VkVertexInputBindingDescription> CvkModel::Vertex::getBindingDescriptions() {
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> CvkModel::Vertex::getAttributeDescriptions() {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

    // binding, location, format and offset.
    attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
    attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
    attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
    attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

    return attributeDescriptions;
}

void CvkModel::Builder::loadModel(const std::string &filepath) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
        throw std::runtime_error(warn + err);
    }

    vertices.clear();
    indices.clear();

    std::unordered_map<Vertex,uint32_t> uniqueVertices{};

    for (const auto &shape : shapes) {
        for (const auto &index :shape.mesh.indices) {
            Vertex vertex{};
            if(index.vertex_index >= 0) {
                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2],
                };
                // Color is an optional field, so check if the obj file actually has the color coords
                vertex.color = {
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2],
                };
            }
            if(index.normal_index >= 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2],
                };
            }
            if(index.texcoord_index >= 0) {
                vertex.uv = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1],
                };
            }
            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }
            indices.push_back(uniqueVertices[vertex]);
        }

    }
}

} //namespace cvk