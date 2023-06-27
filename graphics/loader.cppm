module;
#include "platform/vulkan.hpp"
export module loader;

static vk::DynamicLoader dynamicLoader;
export VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

// Vulkan Dynamic Loader module. Initializes the default dynamic loader.
export class CVulkanFunctionLoader {
public:
    // Before creating instance, call this to load vkGetInstanceProcAddr.
    static void Load() {
        PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dynamicLoader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
    }

    // After creating instance, call this to get instance-related function pointers.
    static void LoadInstanceFunctions(vk::Instance instance) {
        VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);
    }

    // After creating a device, call this to get device-related function pointers.
    static void LoadDeviceFunctions(vk::Device device) {
        VULKAN_HPP_DEFAULT_DISPATCHER.init(device);
    }
};