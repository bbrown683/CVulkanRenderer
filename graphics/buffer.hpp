#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

export class CVulkanBuffer {
    vk::UniqueBuffer buffer;
    vk::UniqueDeviceMemory memory;
public:
    CVulkanBuffer(vk::Device device, vk::PhysicalDeviceMemoryProperties memoryProperties, vk::MemoryPropertyFlags desiredPropertyFlags, vk::BufferUsageFlags usage, void* data, vk::DeviceSize dataSize);

    vk::Buffer GetVkBuffer();

private:
    uint32_t GetMemoryTypeIndex(vk::PhysicalDeviceMemoryProperties memoryProperties, uint32_t memoryTypeBits, vk::MemoryPropertyFlags desiredPropertyFlags);
};