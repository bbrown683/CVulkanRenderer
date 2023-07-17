#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <glm/glm.hpp>

// Vertex Properties.
struct CVulkanVertex {
    glm::vec2 position;
    glm::vec3 color;

    static std::vector<vk::VertexInputAttributeDescription> GetVkVertexInputAttributeDescriptions() {
        return {
            vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32Sfloat, offsetof(CVulkanVertex, position)),
            vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(CVulkanVertex, color)),
        };
    }

    static vk::VertexInputBindingDescription GetVkVertexInputBindingDecription() {
        return vk::VertexInputBindingDescription(0, sizeof(CVulkanVertex), vk::VertexInputRate::eVertex);
    }
};

// PBR Material Properties.
struct CVulkanMaterial {
    float roughness;
    float metallic;
};

struct CVulkanPushConstants {
    glm::mat4 modelViewProjection;
};

// Data used every frame.
struct CVulkanFrame {
    uint32_t currentImage = 0;
    uint32_t currentFrame = 0;
    vk::Extent2D extent;
    vk::Image image;
    vk::ImageView imageView;
    vk::Fence acquireFence;
    vk::Semaphore acquireSemaphore;
    vk::Semaphore submitSemaphore;
};

// Data that can be passed for rendering settings.
struct CVulkanRender {
    std::vector<vk::RenderingAttachmentInfo> colorAttachments;
    std::vector<vk::RenderingAttachmentInfo> depthAttachments;
    std::vector<vk::RenderingAttachmentInfo> stencilAttachments;
};

// Data passed in for draw settings.
struct CVulkanDraw {
    vk::Pipeline pipeline;
    uint32_t verticesCount;
    std::vector<vk::Buffer> vertexBuffers;
    std::vector<vk::DeviceSize> vertexBufferOffsets;
    uint32_t indicesCount;
    vk::Buffer indexBuffer;
    vk::DeviceSize indexBufferOffset;
};