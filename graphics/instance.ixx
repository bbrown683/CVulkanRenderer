module;
#include "platform/vulkan.hpp"
export module instance;
import <vector>;

export struct SDL_Window;

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
    CVulkanInstance(SDL_Window* window);
    vk::Instance GetVkInstance();
    std::vector<vk::PhysicalDevice> GetVkPhysicalDevices();
    std::vector<vk::LayerProperties> GetAvailableVkLayerProperties();
    std::vector<const char*> GetEnabledLayerProperties();
    std::vector<vk::ExtensionProperties> GetAvailableVkExtensionProperties();
    std::vector<const char*> GetEnabledExtensionProperties();
};