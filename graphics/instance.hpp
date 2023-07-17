#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

struct SDL_Window;
class CVulkanDevice;

class CVulkanInstance {
    vk::raii::Context context;
    std::shared_ptr<vk::raii::Instance> instance;
    std::vector<vk::raii::PhysicalDevice> physicalDevices;
#ifdef _DEBUG
    std::unique_ptr<vk::raii::DebugUtilsMessengerEXT> debugUtilsMessenger;
#endif
    std::vector<vk::LayerProperties> availableLayers;
    std::vector<const char*> enabledLayers;
    std::vector<vk::ExtensionProperties> availableExtensions;
    std::vector<const char*> enabledExtensions;
public:
    CVulkanInstance(SDL_Window* window);
    std::shared_ptr<vk::raii::Instance> GetVkInstance();
    std::vector<vk::raii::PhysicalDevice> GetVkPhysicalDevices();
    std::vector<vk::LayerProperties> GetAvailableVkLayerProperties();
    std::vector<const char*> GetEnabledLayerProperties();
    std::vector<vk::ExtensionProperties> GetAvailableVkExtensionProperties();
    std::vector<const char*> GetEnabledExtensionProperties();
    std::unique_ptr<CVulkanDevice> CreateDevice();
private:
    vk::raii::PhysicalDevice SelectPrimaryPhysicalDevice(std::vector<vk::raii::PhysicalDevice> physicalDevices);
};