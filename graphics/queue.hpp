#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

class CVulkanCommandBuffer;
class CVulkanCommandPool;

class CVulkanQueue {
    std::shared_ptr<vk::raii::Device> device;
    std::shared_ptr<vk::raii::Queue> queue;
    uint32_t familyIndex;
public:
    CVulkanQueue(std::shared_ptr<vk::raii::Device> device, uint32_t familyIndex);
    ~CVulkanQueue();
    void Submit(std::shared_ptr<CVulkanCommandBuffer> commandBuffer, vk::Semaphore submitSemaphore = nullptr,
        vk::Semaphore waitSemaphore = nullptr, vk::PipelineStageFlags waitSemaphoreFlags = {},
        vk::Fence signalFence = nullptr);
    CVulkanCommandPool CreateCommandPool(vk::CommandPoolCreateFlags flags = {});
    std::shared_ptr<vk::raii::Queue> GetVkQueue();
    uint32_t GetFamilyIndex();
};