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

export struct CVulkanMesh {
    std::vector<CVulkanVertex> vertices;
    std::vector<uint16_t> indices;
    CVulkanMaterial material;
    CVulkanBuffer vertexBuffer;
    CVulkanBuffer indexBuffer;
    std::vector<CVulkanTexture> textures;    
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
    CVulkanQueue* transferQueue;
public:
    CVulkanMeshLoader(CVulkanDevice* device, CVulkanQueue* transferQueue, std::vector<CVulkanCommandBuffer*> transferCommandBuffers) : device(device), transferQueue(transferQueue) {}

    std::vector<CVulkanMesh> Load(std::string filename) {
        return {};
    }

    CVulkanMesh Load(std::vector<CVulkanVertex> vertices, std::vector<uint16_t> indices = {}, std::vector<CVulkanTexture> textures = {}) {
        auto vkDevice = device->GetVkDevice();
        auto vkMemoryProperties = device->GetVkPhysicalDeviceMemoryProperties();

        vk::DeviceSize vertexBufferSize = sizeof(CVulkanVertex) * vertices.size();
        CVulkanBuffer stagingVertexBuffer(vkDevice, vkMemoryProperties, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, vk::BufferUsageFlagBits::eTransferSrc, vertices.data(), vertexBufferSize);
        CVulkanBuffer vertexBuffer(vkDevice, vkMemoryProperties, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, nullptr, vertexBufferSize);

        if(indices.size() > 0) {
            vk::DeviceSize indexBufferSize = sizeof(uint16_t) * indices.size();
            CVulkanBuffer stagingIndexBuffer(vkDevice, vkMemoryProperties, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, vk::BufferUsageFlagBits::eTransferSrc, indices.data(), indexBufferSize);
            CVulkanBuffer indexBuffer(vkDevice, vkMemoryProperties, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, nullptr, indexBufferSize);
        }
        return {};
    }
};

export class CVulkanMeshRenderer {

};