module;
#include "platform/vulkan.hpp"
export module cmd;
import <optional>;

export class CVulkanCommandPool {
    vk::Device device;
    vk::UniqueCommandPool commandPool;
public:
    CVulkanCommandPool() = default;
    CVulkanCommandPool(vk::Device device, uint8_t queueFamilyIndex) : device(device) {
        commandPool = device.createCommandPoolUnique(vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueFamilyIndex));
    }

    void Reset() {
        device.resetCommandPool(*commandPool);
    }

    vk::CommandPool GetVkCommandPool() {
        return *commandPool;
    }
};

export class CVulkanCommandBuffer {
    vk::Device device;
    vk::UniqueCommandBuffer commandBuffer;
    vk::UniqueFence fence;
public:
    CVulkanCommandBuffer() = default;
    CVulkanCommandBuffer(vk::Device device, vk::CommandPool commandPool) : device(device) {
        auto commandBufferInfo = vk::CommandBufferAllocateInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1);
        commandBuffer = std::move(device.allocateCommandBuffersUnique(commandBufferInfo).front());
    }

    vk::CommandBuffer GetVkCommandBuffer() {
        return *commandBuffer;
    }
};
