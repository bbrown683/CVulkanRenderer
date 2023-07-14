module;
#include "platform/vulkan.hpp"
#include "platform/SDL.hpp"
export module instance;
import <vector>;
import loader;
import device;

export class CVulkanInstance {
    vk::UniqueInstance instance;
    std::vector<vk::PhysicalDevice> physicalDevices;
#ifdef _DEBUG
    vk::UniqueDebugUtilsMessengerEXT debugUtilsMessenger;
#endif
    std::vector<vk::LayerProperties> availableLayers;
    std::vector<const char*> enabledLayers;
    std::vector<vk::ExtensionProperties> availableExtensions;
    std::vector<const char*> enabledExtensions;
public:
    CVulkanInstance(SDL_Window* window) {
        unsigned int extension_count;
        if(!SDL_Vulkan_GetInstanceExtensions(window, &extension_count, nullptr)) {
            printf("CVulkanInstance::CVulkanInstance: Could not get the number of required instance extensions from SDL.");
        }
        enabledExtensions = std::vector<const char*>(extension_count);
        if(!SDL_Vulkan_GetInstanceExtensions(window, &extension_count, enabledExtensions.data())) {
            printf("CVulkanInstance::CVulkanInstance: Could not get the names of required instance extensions from SDL.");
        }
#if _DEBUG
        // Add validation layers.
        enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
        enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

        CVulkanFunctionLoader::Load();

        vk::ApplicationInfo applicationInfo("CVulkan", VK_MAKE_VERSION(1, 0, 0), "CVulkanRenderer", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_3);
        vk::InstanceCreateInfo instanceInfo({}, &applicationInfo, static_cast<uint32_t>(enabledLayers.size()), enabledLayers.data(), static_cast<uint32_t>(enabledExtensions.size()), enabledExtensions.data());

        availableLayers = vk::enumerateInstanceLayerProperties();
        availableExtensions = vk::enumerateInstanceExtensionProperties();
        for(auto& layer : availableLayers) {
            auto layer_extensions = vk::enumerateInstanceExtensionProperties(vk::Optional<const std::string>(layer.layerName));
            if(layer_extensions.size() > 0) {
                availableExtensions.insert(std::end(availableExtensions), std::begin(layer_extensions), std::end(layer_extensions));
            }
        }

        // Create the Vulkan instance. 
        instance = vk::createInstanceUnique(instanceInfo);
        CVulkanFunctionLoader::LoadInstanceFunctions(*instance);

#ifdef _DEBUG
        // Setup debug utils.
        auto debugUtilsInfo = vk::DebugUtilsMessengerCreateInfoEXT();
        debugUtilsInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
            | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
            | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
            | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;
        debugUtilsInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
            | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
            | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
        debugUtilsInfo.pfnUserCallback = debugUtilsCallback;
        debugUtilsMessenger = instance->createDebugUtilsMessengerEXTUnique(debugUtilsInfo);
#endif
        physicalDevices = instance->enumeratePhysicalDevices();
    }

#ifdef _DEBUG
    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugUtilsCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
        vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverityEnum = vk::DebugUtilsMessageSeverityFlagBitsEXT(messageSeverity);
        const char* message = pCallbackData->pMessage;
        switch(messageSeverityEnum) {
            using enum vk::DebugUtilsMessageSeverityFlagBitsEXT;
            case eVerbose: printf("VERBOSE: %s\n", message); break;
            case eInfo: printf("INFO: %s\n", message); break;
            case eWarning: printf("WARNING: %s\n", message); break;
            case eError: printf("ERROR: %s\n", message); break;
        }
        return VK_FALSE;
    }
#endif

    vk::Instance GetVkInstance() {
        return *instance;
    }

    std::vector<vk::PhysicalDevice> GetVkPhysicalDevices() {
        return physicalDevices;
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

    std::unique_ptr<CVulkanDevice> CreateDevice() {
        return std::make_unique<CVulkanDevice>(SelectPrimaryPhysicalDevice(physicalDevices));
    }

private:
    vk::PhysicalDevice SelectPrimaryPhysicalDevice(std::vector<vk::PhysicalDevice> physicalDevices) {
        for(auto& physicalDevice : physicalDevices) {
            vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
            if(properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
                return physicalDevice;
            }
        }
        return physicalDevices.front();
    }
};