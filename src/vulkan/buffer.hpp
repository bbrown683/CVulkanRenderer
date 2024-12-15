#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

enum CVulkanBufferCreationError {
    BUFFER_INVALID_MEMORY_TYPE
};

class CVulkanBufferCreationException : public std::exception {
    CVulkanBufferCreationError error;
public:
    CVulkanBufferCreationException(CVulkanBufferCreationError error) : error(error) {}

    char* what() {
        return (char*)"Failed to create buffer due to error: " + error;
    }

    CVulkanBufferCreationError GetError() {
        return error;
    }
};

class CVulkanBuffer {
    std::unique_ptr<vk::raii::Buffer> buffer;
    std::unique_ptr<vk::raii::DeviceMemory> memory;
    vk::DeviceSize size;
public:
    CVulkanBuffer(std::shared_ptr<vk::raii::Device> device, vk::PhysicalDeviceMemoryProperties memoryProperties, vk::MemoryPropertyFlags desiredPropertyFlags, vk::BufferUsageFlags usage, void* data, vk::DeviceSize dataSize);
    vk::Buffer GetVkBuffer();
    vk::DeviceSize GetVkDeviceSize();
};