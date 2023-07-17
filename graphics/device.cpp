#include "device.hpp"

#include "queue.hpp"
#include "buffer.hpp"
#include "pipeline.hpp"

CVulkanDevice::CVulkanDevice(vk::raii::PhysicalDevice physicalDevice) : physicalDevice(physicalDevice) {
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
    device = std::make_shared<vk::raii::Device>(physicalDevice.createDevice(deviceInfo));
}

CVulkanDevice::~CVulkanDevice() {
    device->waitIdle();
}

vk::SampleCountFlags CVulkanDevice::GetMaximumSupportedMultisamping() {
    return std::min(limits.framebufferColorSampleCounts, limits.framebufferDepthSampleCounts);
}

std::shared_ptr<vk::raii::Device> CVulkanDevice::GetVkDevice() {
    return device;
}

vk::PhysicalDevice CVulkanDevice::GetVkPhysicalDevice() {
    return *physicalDevice;
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

std::vector<const char*> CVulkanDevice::GetEnabledLayerProperties() {
    return enabledLayers;
}

std::vector<vk::ExtensionProperties> CVulkanDevice::GetAvailableVkExtensionProperties() {
    return availableExtensions;
}

std::vector<const char*> CVulkanDevice::GetEnabledExtensionProperties() {
    return enabledExtensions;
}

std::unique_ptr<CVulkanQueue> CVulkanDevice::GetGraphicsQueue() {
    return std::make_unique<CVulkanQueue>(device, graphicsQueueIndex);
}

std::unique_ptr<CVulkanQueue> CVulkanDevice::GetComputeQueue() {
    return std::make_unique<CVulkanQueue>(device, computeQueueIndex);
}

std::unique_ptr<CVulkanQueue> CVulkanDevice::GetTransferQueue() {
    return std::make_unique<CVulkanQueue>(device, transferQueueIndex);
}

CVulkanBuffer CVulkanDevice::CreateBuffer(vk::MemoryPropertyFlags desiredPropertyFlags, vk::BufferUsageFlags usage, void* data, vk::DeviceSize dataSize) {
    return CVulkanBuffer(device, memoryProperties, desiredPropertyFlags, usage, data, dataSize);
}

CVulkanGraphicsPipeline CVulkanDevice::CreateGraphicsPipeline(std::string vertexShaderFile, std::string fragmentShaderFile, vk::Format colorFormat) {
    return CVulkanGraphicsPipeline(device, vertexShaderFile, fragmentShaderFile, colorFormat);
}
