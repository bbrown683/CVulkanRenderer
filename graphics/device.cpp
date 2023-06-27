module;
#include "platform/vulkan.hpp"
module device;
import loader;

CVulkanDevice::CVulkanDevice(vk::PhysicalDevice physicalDevice) : physicalDevice(physicalDevice) {
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

    auto physicalDeviceFeatures = vk::PhysicalDeviceFeatures()
        .setFillModeNonSolid(true)
        .setSamplerAnisotropy(true);

    std::vector<vk::DeviceQueueCreateInfo> queueInfos = { graphicsInfo, computeInfo, transferInfo };
    std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    auto deviceInfo = vk::DeviceCreateInfo(vk::DeviceCreateFlags(), queueInfos, nullptr, deviceExtensions, &physicalDeviceFeatures);
    device = physicalDevice.createDeviceUnique(deviceInfo);
    CVulkanFunctionLoader::LoadDeviceFunctions(*device);
}

vk::Device CVulkanDevice::GetVkDevice() {
    return *device;
}

vk::PhysicalDevice CVulkanDevice::GetVkPhysicalDevice() {
    return physicalDevice;
}

vk::PhysicalDeviceProperties CVulkanDevice::GetVkPhysicalDeviceProperties() {
    return properties;
}

vk::PhysicalDeviceMemoryProperties CVulkanDevice::GetVkPhysicalDeviceMemoryProperties() {
    return memoryProperties;
}

std::vector<vk::LayerProperties> CVulkanDevice::GetAvailableVkLayerProperties() {
    return availableLayers;
}

std::vector<vk::LayerProperties> CVulkanDevice::GetEnabledVkLayerProperties() {
    return enabledLayers;
}

std::vector<vk::ExtensionProperties> CVulkanDevice::GetAvailableVkExtensionProperties() {
    return availableExtensions;
}

std::vector<vk::ExtensionProperties> CVulkanDevice::GetEnabledVkExtensionProperties() {
    return enabledExtensions;
}

uint8_t CVulkanDevice::GetGraphicsQueueIndex() {
    return graphicsQueueIndex;
}

uint8_t CVulkanDevice::GetComputeQueueIndex() {
    return computeQueueIndex;
}

uint8_t CVulkanDevice::GetTransferQueueIndex() {
    return transferQueueIndex;
}