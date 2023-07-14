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
import texture;
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
    CVulkanCommandBuffer* commandBuffer;
public:
    CVulkanMeshLoader(CVulkanDevice* device, CVulkanQueue* queue, CVulkanCommandPool* commandPool, CVulkanCommandBuffer* commandBuffer)
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

    void Render(CVulkanFrame frame, std::vector<std::shared_ptr<CVulkanMesh>> meshes) {
        CVulkanRender render;
        render.colorAttachments = { vk::RenderingAttachmentInfo(frame.imageView, vk::ImageLayout::eColorAttachmentOptimal,
                                                            vk::ResolveModeFlagBits::eNone, nullptr, vk::ImageLayout::eUndefined,
                                                            vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
                                                           vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)) };
        /*
        render.setDepthAttachments = { vk::RenderingAttachmentInfo({}, vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                                                     vk::ResolveModeFlagBits::eNone, {}, vk::ImageLayout::eUndefined,
                                                                     vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
                                                                     vk::ClearDepthStencilValue(1.0f, 0)) };
        */
        render.pipeline = pipeline->GetVkPipeline();

        for(auto& mesh : meshes) {
            render.verticesCount = static_cast<uint32_t>(mesh->vertices.size());
            render.vertexBuffers = { mesh->vertexBuffer->GetVkBuffer() };
            render.vertexBufferOffsets = { 0 };
            render.indicesCount = static_cast<uint32_t>(mesh->indices.size());
            render.indexBuffer = mesh->indexBuffer->GetVkBuffer();
            render.indexBufferOffset = 0;
            commandBuffers[frame.currentFrame]->Render(frame, render);
        }
    }
};