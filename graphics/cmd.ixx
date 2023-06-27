module;
#include "platform/vulkan.hpp"
export module cmd;
import <optional>;

export class CVulkanCommandPool {
    vk::Device device;
    vk::UniqueCommandPool commandPool;
public:
    CVulkanCommandPool(vk::Device device, uint8_t queueFamilyIndex);
    void Reset();
    vk::CommandPool GetVkCommandPool();
};

export class CVulkanCommandBuffer {
    vk::Device device;
    vk::UniqueCommandBuffer commandBuffer;
    vk::UniqueFence fence;
public:
    CVulkanCommandBuffer(vk::Device device, vk::CommandPool commandPool);
    vk::CommandBuffer GetVkCommandBuffer();
};
