module;
#include "platform/vulkan.hpp"
module cmd;

CVulkanCommandPool::CVulkanCommandPool(vk::Device device, uint8_t queueFamilyIndex) : device(device) {
    commandPool = device.createCommandPoolUnique(vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueFamilyIndex));
}

void CVulkanCommandPool::Reset() {
    device.resetCommandPool(*commandPool);
}

vk::CommandPool CVulkanCommandPool::GetVkCommandPool() {
    return *commandPool;
}

CVulkanCommandBuffer::CVulkanCommandBuffer(vk::Device device, vk::CommandPool commandPool) : device(device) {
    auto commandBufferInfo = vk::CommandBufferAllocateInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1);
    commandBuffer = std::move(device.allocateCommandBuffersUnique(commandBufferInfo).front());
}

vk::CommandBuffer CVulkanCommandBuffer::GetVkCommandBuffer() {
    return *commandBuffer;
}