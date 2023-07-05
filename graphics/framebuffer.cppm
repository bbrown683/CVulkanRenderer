module;
#include "platform/vulkan.hpp"
export module framebuffer;

export class CVulkanFramebuffer {
    vk::UniqueImageView imageView;
    vk::UniqueFramebuffer framebuffer;
public:
    CVulkanFramebuffer() = default;
    CVulkanFramebuffer(vk::Device device, vk::RenderPass renderPass, vk::Image image, vk::Format format, vk::Extent2D extent, uint32_t baseMipLevel = 0, uint32_t mipLevels = 1) {
        auto imageSubresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, baseMipLevel, mipLevels, 0, 1);
        auto imageViewInfo = vk::ImageViewCreateInfo(vk::ImageViewCreateFlags(), image, vk::ImageViewType::e2D, format, {}, imageSubresourceRange);
        imageView = device.createImageViewUnique(imageViewInfo);

        auto framebufferInfo = vk::FramebufferCreateInfo(vk::FramebufferCreateFlags(), renderPass, *imageView, extent.width, extent.height, 1);
        framebuffer = device.createFramebufferUnique(framebufferInfo);
    }

    vk::Framebuffer GetVkFramebuffer() {
        return *framebuffer;
    }

    vk::ImageView GetVkImageView() {
        return *imageView;
    }
};
