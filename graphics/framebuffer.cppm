module;
#include "platform/vulkan.hpp"
export module framebuffer;

export class CVulkanFramebuffer {
    vk::Device device;
    vk::UniqueFramebuffer framebuffer;
public:
    CVulkanFramebuffer() = default;
    CVulkanFramebuffer(vk::Device device, vk::RenderPass renderPass, vk::Image image, vk::Format format, vk::Extent2D extent) : device(device) {

    }
};
