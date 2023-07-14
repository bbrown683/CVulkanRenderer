module;
#include "platform/vulkan.hpp"
export module types;

// Data used every frame.
export struct CVulkanFrame {
    uint32_t currentImage;
    uint32_t currentFrame;
    vk::Extent2D extent;
    vk::Image image;
    vk::ImageView imageView;
    vk::Fence acquireFence;
    vk::Semaphore acquireSemaphore;
    vk::Semaphore submitSemaphore;
};

// Data that can be passed into a draw call.
export struct CVulkanRender {
    vk::Pipeline pipeline;
    std::vector<vk::RenderingAttachmentInfo> colorAttachments;
    std::vector<vk::RenderingAttachmentInfo> depthAttachments;
    std::vector<vk::RenderingAttachmentInfo> stencilAttachments;
    uint32_t verticesCount;
    std::vector<vk::Buffer> vertexBuffers;
    std::vector<vk::DeviceSize> vertexBufferOffsets;
    uint32_t indicesCount;
    vk::Buffer indexBuffer;
    vk::DeviceSize indexBufferOffset;
};