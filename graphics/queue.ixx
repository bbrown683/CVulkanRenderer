module;
#include "platform/vulkan.hpp"
export module queue;
import <optional>;

export class CVulkanQueue {
    vk::Device device;
    vk::Queue queue;
    uint8_t familyIndex;
    vk::UniqueSemaphore submitSemaphore;
public:
    CVulkanQueue(vk::Device device, uint8_t familyIndex);
    void Submit(vk::CommandBuffer commandBuffer, std::optional<vk::Semaphore> waitSemaphore = std::nullopt, std::optional<vk::Fence> signalFence = std::nullopt);
    vk::Queue GetVkQueue();
    uint8_t GetFamilyIndex();
};