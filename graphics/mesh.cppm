module;
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "platform/vulkan.hpp"
export module mesh;
import device;
import cmd;
import queue;
import pipeline;
import buffer;
import image;
import pipeline;
import types;

export struct CVulkanMesh {
public:
    std::vector<CVulkanVertex> vertices;
    std::vector<uint16_t> indices;
    CVulkanMaterial material;
    std::unique_ptr<CVulkanBuffer> vertexBuffer;
    std::unique_ptr<CVulkanBuffer> indexBuffer;
    //std::vector<std::unique_ptr<CVulkanTexture>> textures;  
    glm::mat4 worldTransform;

    void SetLocation(glm::vec3 location) {
        worldTransform = glm::translate(worldTransform, location);
    }

    void SetRotation(glm::vec3 rotation) {
        worldTransform = glm::transpose(glm::rotate(worldTransform, glm::radians(rotation.x), glm::vec3(1, 0, 0)) *
            glm::rotate(worldTransform, glm::radians(rotation.y), glm::vec3(0, 1, 0)) *
            glm::rotate(worldTransform, glm::radians(rotation.z), glm::vec3(0, 0, 1)));
    }

    void SetScale(glm::vec3 scale) {
        worldTransform = glm::scale(worldTransform, scale);
    }
};

export class CVulkanMeshLoader {
    CVulkanDevice* device;
    CVulkanQueue* queue;
    CVulkanCommandPool* commandPool;
    std::shared_ptr<CVulkanCommandBuffer> commandBuffer;
public:
    CVulkanMeshLoader(CVulkanDevice* device, CVulkanQueue* queue, CVulkanCommandPool* commandPool, std::shared_ptr<CVulkanCommandBuffer> commandBuffer)
        : device(device), queue(queue), commandPool(commandPool), commandBuffer(commandBuffer) {}

    std::vector<CVulkanMesh> Load(std::string filename) {
        return {};
    }

    CVulkanMesh Load(std::vector<CVulkanVertex> vertices, std::vector<uint16_t> indices = {}) {
        auto vkDevice = device->GetVkDevice();
        auto vkMemoryProperties = device->GetVkPhysicalDeviceMemoryProperties();

        CVulkanMesh mesh;
        mesh.vertices = vertices;
        mesh.indices = indices;

        vk::DeviceSize vertexBufferSize = sizeof(CVulkanVertex) * vertices.size();
        auto stagingVertexBuffer = device->CreateBuffer(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, vk::BufferUsageFlagBits::eTransferSrc, vertices.data(), vertexBufferSize);
        mesh.vertexBuffer = std::make_unique<CVulkanBuffer>(device->CreateBuffer(vk::MemoryPropertyFlagBits::eDeviceLocal, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, nullptr, vertexBufferSize));

        commandBuffer->CopyBuffer(&stagingVertexBuffer, mesh.vertexBuffer.get(), vk::BufferCopy(0, 0, vertexBufferSize));
        queue->Submit(commandBuffer);
        commandPool->Reset();

        if(indices.size() > 0) {
            vk::DeviceSize indexBufferSize = sizeof(uint16_t) * indices.size();
            auto stagingIndexBuffer = device->CreateBuffer(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, vk::BufferUsageFlagBits::eTransferSrc, indices.data(), indexBufferSize);
            mesh.indexBuffer = std::make_unique<CVulkanBuffer>(device->CreateBuffer(vk::MemoryPropertyFlagBits::eDeviceLocal, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, nullptr, indexBufferSize));
        
            commandBuffer->CopyBuffer(&stagingIndexBuffer, mesh.indexBuffer.get(), vk::BufferCopy(0, 0, indexBufferSize));
            queue->Submit(commandBuffer);
            commandPool->Reset();
        }
        return mesh;
    }
};

export class CVulkanMeshRenderer {
    CVulkanGraphicsPipeline* pipeline;
    std::vector<std::shared_ptr<CVulkanCommandBuffer>> commandBuffers;
public:
    CVulkanMeshRenderer(CVulkanGraphicsPipeline* pipeline, std::vector<std::shared_ptr<CVulkanCommandBuffer>> commandBuffers) 
        : pipeline(pipeline), commandBuffers(commandBuffers) {}

    void Draw(CVulkanFrame* frame, std::vector<std::shared_ptr<CVulkanMesh>> meshes) {
        CVulkanDraw draw;
        draw.pipeline = pipeline->GetVkPipeline();
        for(auto& mesh : meshes) {
            draw.verticesCount = static_cast<uint32_t>(mesh->vertices.size());
            draw.vertexBuffers = { mesh->vertexBuffer->GetVkBuffer() };
            draw.vertexBufferOffsets = { 0 };
            draw.indicesCount = static_cast<uint32_t>(mesh->indices.size());
            draw.indexBuffer = mesh->indexBuffer->GetVkBuffer();
            draw.indexBufferOffset = 0;
            commandBuffers[frame->currentFrame]->Draw(&draw);
        }
    }
};