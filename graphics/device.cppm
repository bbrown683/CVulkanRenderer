module;
#include "platform/vulkan.hpp"
export module device;
import <algorithm>;
import loader;

export class CVulkanDevice {
    vk::UniqueDevice device;
    vk::PhysicalDevice physicalDevice;
    vk::PhysicalDeviceProperties properties;
    vk::PhysicalDeviceFeatures features;
    vk::PhysicalDeviceLimits limits;
    vk::PhysicalDeviceMemoryProperties memoryProperties;
    uint32_t computeQueueIndex;
    uint32_t graphicsQueueIndex;
    uint32_t transferQueueIndex;
    std::vector<vk::LayerProperties> availableLayers;
    std::vector<const char*> enabledLayers;
    std::vector<vk::ExtensionProperties> availableExtensions;
    std::vector<const char*> enabledExtensions;
public:
    CVulkanDevice() = default;
    CVulkanDevice(vk::PhysicalDevice physicalDevice) : physicalDevice(physicalDevice) {
        availableLayers = physicalDevice.enumerateDeviceLayerProperties();
        availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();
        for(auto& layer : availableLayers) {
            auto layer_extensions = physicalDevice.enumerateDeviceExtensionProperties(vk::Optional<const std::string>(layer.layerName));
            if(layer_extensions.size() > 0) {
                availableExtensions.insert(std::end(availableExtensions), std::begin(layer_extensions), std::end(layer_extensions));
            }
        }

        properties = physicalDevice.getProperties();
        limits = properties.limits;
        features = physicalDevice.getFeatures();
        memoryProperties = physicalDevice.getMemoryProperties();
        auto queueFamilies = physicalDevice.getQueueFamilyProperties();

        graphicsQueueIndex = 0;
        computeQueueIndex = 0;
        transferQueueIndex = 0;
        for(auto i = 0; auto & queueFamily : queueFamilies) {
            if(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
                graphicsQueueIndex = i;
            }
            if(queueFamily.queueFlags & vk::QueueFlagBits::eCompute) {
                computeQueueIndex = i;
            }
            if(queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) {
                transferQueueIndex = i;
            }
            i++;
        }

        std::vector<float> priorities = { 1.0f };
        vk::DeviceQueueCreateInfo graphicsInfo({}, graphicsQueueIndex, priorities);
        vk::DeviceQueueCreateInfo computeInfo({}, computeQueueIndex, priorities);
        vk::DeviceQueueCreateInfo transferInfo({}, transferQueueIndex, priorities);

        vk::PhysicalDeviceFeatures defaultPhysicalDeviceFeatures;
        defaultPhysicalDeviceFeatures.setFillModeNonSolid(true);
        defaultPhysicalDeviceFeatures.setSamplerAnisotropy(true);

        // Enable Dynamic Rendering features.
        vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures(true);
        vk::PhysicalDeviceFeatures2 deviceFeatures(defaultPhysicalDeviceFeatures, &dynamicRenderingFeatures);

        std::vector<vk::DeviceQueueCreateInfo> queueInfos = { graphicsInfo, computeInfo, transferInfo };
        std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME };
        vk::DeviceCreateInfo deviceInfo({}, queueInfos, nullptr, deviceExtensions, nullptr, &deviceFeatures);
        device = physicalDevice.createDeviceUnique(deviceInfo);
        CVulkanFunctionLoader::LoadDeviceFunctions(*device);
    }

    ~CVulkanDevice() {
        device->waitIdle(); // Wait for all operations to complete before shutting down.
    }

    vk::Device operator*() const {
        return *device;
    }

    vk::SampleCountFlags GetMaximumSupportedMultisamping() {
        return std::min(limits.framebufferColorSampleCounts, limits.framebufferDepthSampleCounts);
    }

    vk::Device GetVkDevice() {
        return *device;
    }

    vk::PhysicalDevice GetVkPhysicalDevice() {
        return physicalDevice;
    }

    vk::PhysicalDeviceProperties GetVkPhysicalDeviceProperties() {
        return properties;
    }

    vk::PhysicalDeviceMemoryProperties GetVkPhysicalDeviceMemoryProperties() {
        return memoryProperties;
    }

    std::vector<vk::LayerProperties> GetAvailableVkLayerProperties() {
        return availableLayers;
    }

    std::vector<const char*> GetEnabledLayerProperties() {
        return enabledLayers;
    }

    std::vector<vk::ExtensionProperties> GetAvailableVkExtensionProperties() {
        return availableExtensions;
    }

    std::vector<const char*> GetEnabledExtensionProperties() {
        return enabledExtensions;
    }

    uint32_t GetGraphicsQueueIndex() {
        return graphicsQueueIndex;
    }
    
    uint32_t GetComputeQueueIndex() {
        return computeQueueIndex;
    }

    uint32_t GetTransferQueueIndex() {
        return transferQueueIndex;
    }
};
