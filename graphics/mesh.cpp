#include "mesh.hpp"

#include "device.hpp"
#include "queue.hpp"
#include "buffer.hpp"
#include "pipeline.hpp"
#include "cmd.hpp"
#include "types.hpp"

CVulkanMeshLoader::CVulkanMeshLoader(CVulkanDevice* device, CVulkanQueue* transferQueue, std::shared_ptr<CVulkanCommandBuffer> transferCommandBuffer) 
    : device(device), transferQueue(transferQueue), transferCommandBuffer(transferCommandBuffer) {}

CVulkanMesh CVulkanMeshLoader::Load(std::vector<CVulkanVertex> vertices, std::vector<uint16_t> indices) {
    auto vkDevice = device->GetVkDevice();
    auto vkMemoryProperties = device->GetVkPhysicalDeviceMemoryProperties();

    CVulkanMesh mesh;
    mesh.vertices = vertices;
    mesh.indices = indices;

    vk::DeviceSize vertexBufferSize = sizeof(CVulkanVertex) * vertices.size();
    auto stagingVertexBuffer = device->CreateBuffer(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, vk::BufferUsageFlagBits::eTransferSrc, vertices.data(), vertexBufferSize);
    mesh.vertexBuffer = std::make_unique<CVulkanBuffer>(device->CreateBuffer(vk::MemoryPropertyFlagBits::eDeviceLocal, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, nullptr, vertexBufferSize));

    transferCommandBuffer->CopyBuffer(&stagingVertexBuffer, mesh.vertexBuffer.get(), vk::BufferCopy(0, 0, vertexBufferSize));
    transferQueue->Submit(transferCommandBuffer);
    transferCommandBuffer->Reset();

    if(indices.size() > 0) {
        vk::DeviceSize indexBufferSize = sizeof(uint16_t) * indices.size();
        auto stagingIndexBuffer = device->CreateBuffer(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, vk::BufferUsageFlagBits::eTransferSrc, indices.data(), indexBufferSize);
        mesh.indexBuffer = std::make_unique<CVulkanBuffer>(device->CreateBuffer(vk::MemoryPropertyFlagBits::eDeviceLocal, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, nullptr, indexBufferSize));

        transferCommandBuffer->CopyBuffer(&stagingIndexBuffer, mesh.indexBuffer.get(), vk::BufferCopy(0, 0, indexBufferSize));
        transferQueue->Submit(transferCommandBuffer);
        transferCommandBuffer->Reset();
    }
    return mesh;
}

CVulkanMeshRenderer::CVulkanMeshRenderer(CVulkanGraphicsPipeline* pipeline, std::vector<std::shared_ptr<CVulkanCommandBuffer>> graphicsCommandBuffers)
    : pipeline(pipeline), graphicsCommandBuffers(graphicsCommandBuffers) {}

void CVulkanMeshRenderer::Draw(CVulkanFrame* frame, std::vector<std::shared_ptr<CVulkanMesh>> meshes) {
    CVulkanDraw draw;
    draw.pipeline = pipeline->GetVkPipeline();
    for(auto& mesh : meshes) {
        draw.verticesCount = static_cast<uint32_t>(mesh->vertices.size());
        draw.vertexBuffers = { mesh->vertexBuffer->GetVkBuffer() };
        draw.vertexBufferOffsets = { 0 };
        draw.indicesCount = static_cast<uint32_t>(mesh->indices.size());
        draw.indexBuffer = mesh->indexBuffer->GetVkBuffer();
        draw.indexBufferOffset = 0;
        graphicsCommandBuffers[frame->currentFrame]->Draw(&draw);
    }
}
