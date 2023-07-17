#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct CVulkanVertex;
struct CVulkanMaterial;
class CVulkanBuffer;
class CVulkanDevice;
class CVulkanQueue;
class CVulkanCommandPool;
class CVulkanCommandBuffer;
class CVulkanGraphicsPipeline;
struct CVulkanFrame;

struct CVulkanMesh {
    std::vector<CVulkanVertex> vertices;
    std::vector<uint16_t> indices;
    std::vector<CVulkanMaterial> material;
    std::unique_ptr<CVulkanBuffer> vertexBuffer;
    std::unique_ptr<CVulkanBuffer> indexBuffer;
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

class CVulkanMeshLoader {
    CVulkanDevice* device;
    CVulkanQueue* queue;
    CVulkanCommandPool* commandPool;
    std::shared_ptr<CVulkanCommandBuffer> commandBuffer;
public:
    CVulkanMeshLoader(CVulkanDevice* device, CVulkanQueue* queue, CVulkanCommandPool* commandPool, std::shared_ptr<CVulkanCommandBuffer> commandBuffer);
    CVulkanMesh Load(std::vector<CVulkanVertex> vertices, std::vector<uint16_t> indices = {});
};

class CVulkanMeshRenderer {
    CVulkanGraphicsPipeline* pipeline;
    std::vector<std::shared_ptr<CVulkanCommandBuffer>> commandBuffers;
public:
    CVulkanMeshRenderer(CVulkanGraphicsPipeline* pipeline, std::vector<std::shared_ptr<CVulkanCommandBuffer>> commandBuffers);
    void Draw(CVulkanFrame* frame, std::vector<std::shared_ptr<CVulkanMesh>> meshes);
};