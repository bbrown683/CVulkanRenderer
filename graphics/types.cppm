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
    vk::Image viewportImage;
    vk::ImageView viewportImageView;
    vk::Fence acquireFence;
    vk::Semaphore acquireSemaphore;
    vk::Semaphore submitSemaphore;
};

// Data that can be passed for rendering settings.
export struct CVulkanRender {
    std::vector<vk::RenderingAttachmentInfo> colorAttachments;
    std::vector<vk::RenderingAttachmentInfo> depthAttachments;
    std::vector<vk::RenderingAttachmentInfo> stencilAttachments;

};

// Data passed in for draw settings.
export struct CVulkanDraw {
    vk::Pipeline pipeline;
    uint32_t verticesCount;
    std::vector<vk::Buffer> vertexBuffers;
    std::vector<vk::DeviceSize> vertexBufferOffsets;
    uint32_t indicesCount;
    vk::Buffer indexBuffer;
    vk::DeviceSize indexBufferOffset;
};