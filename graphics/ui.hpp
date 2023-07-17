#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

struct SDL_Window;
class CVulkanInstance;
class CVulkanDevice;
class CVulkanQueue;
class CVulkanCommandPool;
class CVulkanCommandBuffer;
struct CVulkanFrame;

class CVulkanUi {
    SDL_Window* window;
    CVulkanInstance* instance;
    CVulkanDevice* device;
    CVulkanQueue* queue;
    CVulkanCommandPool* commandPool;
    std::vector<std::shared_ptr<CVulkanCommandBuffer>> commandBuffers;
    std::unique_ptr<vk::raii::DescriptorPool> descriptorPool;
public:
    CVulkanUi(SDL_Window* window, CVulkanInstance* instance, CVulkanDevice* device, CVulkanQueue* queue,
        CVulkanCommandPool* commandPool, std::vector<std::shared_ptr<CVulkanCommandBuffer>> commandBuffers,
        uint32_t imageCount, vk::Format colorFormat);
    ~CVulkanUi();
    void Draw(CVulkanFrame* frame);
};