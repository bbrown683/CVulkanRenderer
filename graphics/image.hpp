#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <exception>

enum CVulkanImageCreationError {
    IMAGE_LOAD_FAILED,
    IMAGE_INVALID_MEMORY_TYPE
};

class CVulkanImageCreationException : public std::exception {
    CVulkanImageCreationError error;
public:
    CVulkanImageCreationException(CVulkanImageCreationError error) : error(error) {}

    char* what() {
        return (char*)"Failed to create image due to error: " + error;
    }

    CVulkanImageCreationError GetError() {
        return error;
    }
};

class CVulkanBuffer;
class CVulkanDevice;
class CVulkanQueue;
class CVulkanCommandPool;
class CVulkanCommandBuffer;

class CVulkanImage {
    std::shared_ptr<vk::raii::Image> image;
    std::unique_ptr<vk::raii::DeviceMemory> memory;
public:
    // Creates an image from bits.
    CVulkanImage(std::shared_ptr<vk::raii::Device> device, vk::PhysicalDeviceMemoryProperties memoryProperties, vk::Extent3D extent, vk::Format format, uint8_t mipLevels = 1,
        vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1);
    vk::Image GetVkImage();
private:
};

class CVulkanImageLoader {
    CVulkanDevice* device;
    CVulkanQueue* queue;
    CVulkanCommandPool* commandPool;
    std::shared_ptr<CVulkanCommandBuffer> commandBuffer;
public:
    CVulkanImageLoader(CVulkanDevice* device, CVulkanQueue* queue, CVulkanCommandPool* commandPool, std::shared_ptr<CVulkanCommandBuffer> commandBuffer);
    CVulkanImage Load(std::string path, vk::Format format, uint8_t mipLevels, vk::SampleCountFlagBits samples);
};