#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

class CVulkanBuffer;
class CVulkanImage;
class CVulkanGraphicsPipeline;
class CVulkanQueue;

class CVulkanDevice {
    std::shared_ptr<vk::raii::Device> device;
    vk::raii::PhysicalDevice physicalDevice;
    vk::PhysicalDeviceProperties properties;
    vk::PhysicalDeviceFeatures features;
    vk::PhysicalDeviceLimits limits;
    vk::PhysicalDeviceMemoryProperties memoryProperties;
    std::vector<vk::LayerProperties> availableLayers;
    std::vector<const char*> enabledLayers;
    std::vector<vk::ExtensionProperties> availableExtensions;
    std::vector<const char*> enabledExtensions;
    uint32_t graphicsQueueIndex;
    uint32_t computeQueueIndex;
    uint32_t transferQueueIndex;
public:
    CVulkanDevice(vk::raii::PhysicalDevice physicalDevice);
    ~CVulkanDevice();
    vk::SampleCountFlags GetMaximumSupportedMultisamping();
    std::shared_ptr<vk::raii::Device> GetVkDevice();
    vk::PhysicalDevice GetVkPhysicalDevice();
    vk::PhysicalDeviceProperties GetVkPhysicalDeviceProperties();
    vk::PhysicalDeviceMemoryProperties GetVkPhysicalDeviceMemoryProperties();
    std::vector<vk::LayerProperties> GetAvailableVkLayerProperties();
    std::vector<const char*> GetEnabledLayerProperties();
    std::vector<vk::ExtensionProperties> GetAvailableVkExtensionProperties();
    std::vector<const char*> GetEnabledExtensionProperties();
    std::unique_ptr<CVulkanQueue> GetGraphicsQueue();
    std::unique_ptr<CVulkanQueue> GetComputeQueue();
    std::unique_ptr<CVulkanQueue> GetTransferQueue();
    CVulkanBuffer CreateBuffer(vk::MemoryPropertyFlags desiredPropertyFlags, vk::BufferUsageFlags usage, void* data, vk::DeviceSize dataSize);
    CVulkanGraphicsPipeline CreateGraphicsPipeline(std::string vertexShaderFile, std::string fragmentShaderFile, vk::Format colorFormat);
    CVulkanImage CreateImage(vk::Extent3D extent, vk::Format format, uint8_t mipLevels = 1, vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1);
};