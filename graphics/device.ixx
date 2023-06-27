module;
#include "platform/vulkan.hpp"
export module device;

export class CVulkanInstance;

export class CVulkanDevice {
    vk::UniqueDevice device;
    vk::PhysicalDevice physicalDevice;
    vk::PhysicalDeviceProperties properties;
    vk::PhysicalDeviceFeatures features;
    vk::PhysicalDeviceLimits limits;
    vk::PhysicalDeviceMemoryProperties memoryProperties;
    uint8_t computeQueueIndex;
    uint8_t graphicsQueueIndex;
    uint8_t transferQueueIndex;
    std::vector<vk::LayerProperties> availableLayers;
    std::vector<vk::LayerProperties> enabledLayers;
    std::vector<vk::ExtensionProperties> availableExtensions;
    std::vector<vk::ExtensionProperties> enabledExtensions;
public:
    CVulkanDevice(vk::PhysicalDevice physicalDevice);
    vk::Device GetVkDevice();
    vk::PhysicalDevice GetVkPhysicalDevice();
    vk::PhysicalDeviceProperties GetVkPhysicalDeviceProperties();
    vk::PhysicalDeviceMemoryProperties GetVkPhysicalDeviceMemoryProperties();
    std::vector<vk::LayerProperties> GetAvailableVkLayerProperties();
    std::vector<vk::LayerProperties> GetEnabledVkLayerProperties();
    std::vector<vk::ExtensionProperties> GetAvailableVkExtensionProperties();
    std::vector<vk::ExtensionProperties> GetEnabledVkExtensionProperties();
    uint8_t GetGraphicsQueueIndex();
    uint8_t GetComputeQueueIndex();
    uint8_t GetTransferQueueIndex();
};
