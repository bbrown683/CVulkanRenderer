module;
#include "platform/vulkan.hpp"
export module device;
import <algorithm>;
import loader;
import queue;
import buffer;
import pipeline;

export class CVulkanDevice {
    vk::UniqueDevice device;
    vk::PhysicalDevice physicalDevice;
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
    friend class CVulkanDeviceDeleter;
public:
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
        enabledExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME };
        vk::DeviceCreateInfo deviceInfo({}, queueInfos, nullptr, enabledExtensions, nullptr, &deviceFeatures);
        device = physicalDevice.createDeviceUnique(deviceInfo);
        CVulkanFunctionLoader::LoadDeviceFunctions(*device);
    }

    ~CVulkanDevice() {
        device->waitIdle();
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

    CVulkanQueue GetGraphicsQueue() {
        return CVulkanQueue(*device, graphicsQueueIndex);
    }

    CVulkanQueue GetComputeQueue() {
        return CVulkanQueue(*device, computeQueueIndex);
    }

    CVulkanQueue GetTransferQueue() {
        return CVulkanQueue(*device, transferQueueIndex);
    }

    CVulkanGraphicsPipeline CreateGraphicsPipeline(std::string vertexShaderFile, std::string fragmentShaderFile, vk::Format colorFormat) {
        return CVulkanGraphicsPipeline(*device, "vertex.spv", "fragment.spv", colorFormat);
    }

    CVulkanBuffer CreateBuffer(vk::MemoryPropertyFlags desiredPropertyFlags, vk::BufferUsageFlags usage, void* data, vk::DeviceSize dataSize) {
        return CVulkanBuffer(*device, memoryProperties, desiredPropertyFlags, usage, data, dataSize);
    }
};