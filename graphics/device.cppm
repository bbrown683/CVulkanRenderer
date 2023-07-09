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
    uint8_t computeQueueIndex;
    uint8_t graphicsQueueIndex;
    uint8_t transferQueueIndex;
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
        auto graphicsInfo = vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), graphicsQueueIndex, priorities);
        auto computeInfo = vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), computeQueueIndex, priorities);
        auto transferInfo = vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), transferQueueIndex, priorities);

        std::vector<vk::PhysicalDeviceFeatures> features;
        auto defaultPhysicalDeviceFeatures = vk::PhysicalDeviceFeatures()
            .setFillModeNonSolid(true)
            .setSamplerAnisotropy(true);

        // Enable Dynamic Rendering features.
        /*
        auto extendedDynamicState3PhysicalDeviceFeatures = vk::PhysicalDeviceExtendedDynamicState3FeaturesEXT()
            .setExtendedDynamicState3PolygonMode(true)
            .setExtendedDynamicState3RasterizationSamples(true);
        */
        auto enableExtendedDynamicStatePhysicalDeviceFeatures = vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT(true);
        auto dynamicRenderingPhysicalDeviceFeatures = vk::PhysicalDeviceDynamicRenderingFeaturesKHR(true);
        auto deviceFeatures = vk::PhysicalDeviceFeatures2(defaultPhysicalDeviceFeatures, &dynamicRenderingPhysicalDeviceFeatures);

        std::vector<vk::DeviceQueueCreateInfo> queueInfos = { graphicsInfo, computeInfo, transferInfo };
        std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        auto deviceInfo = vk::DeviceCreateInfo({}, queueInfos, nullptr, deviceExtensions, nullptr, &deviceFeatures);
        device = physicalDevice.createDeviceUnique(deviceInfo);
        CVulkanFunctionLoader::LoadDeviceFunctions(*device);
    }

    CVulkanDevice(const CVulkanDevice&) = default;
    CVulkanDevice(CVulkanDevice&&) = default;
    CVulkanDevice& operator=(const CVulkanDevice&) = default;
    CVulkanDevice& operator=(CVulkanDevice&& device) = default;

    ~CVulkanDevice() {
        if(device) {
            device->waitIdle(); // Wait for all operations to complete before shutting down.
        }
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

    uint8_t GetGraphicsQueueIndex() {
        return graphicsQueueIndex;
    }
    
    uint8_t GetComputeQueueIndex() {
        return computeQueueIndex;
    }

    uint8_t GetTransferQueueIndex() {
        return transferQueueIndex;
    }
};
