#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

class CVulkanBuffer {
    std::unique_ptr<vk::raii::Buffer> buffer;
    std::unique_ptr<vk::raii::DeviceMemory> memory;
public:
    CVulkanBuffer(std::shared_ptr<vk::raii::Device> device, vk::PhysicalDeviceMemoryProperties memoryProperties, vk::MemoryPropertyFlags desiredPropertyFlags, vk::BufferUsageFlags usage, void* data, vk::DeviceSize dataSize);
    vk::Buffer GetVkBuffer();
private:
    uint32_t GetMemoryTypeIndex(vk::PhysicalDeviceMemoryProperties memoryProperties, uint32_t memoryTypeBits, vk::MemoryPropertyFlags desiredPropertyFlags);
};