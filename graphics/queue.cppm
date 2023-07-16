module;
#include "platform/vulkan.hpp"
export module queue;
import cmd;

export class CVulkanQueue {
    vk::Device device;
    vk::Queue queue;
    uint32_t familyIndex;
    std::vector<vk::UniqueSemaphore> submitSemaphores;
public:
    CVulkanQueue(vk::Device device, uint32_t familyIndex) : device(device), familyIndex(familyIndex) {
        queue = device.getQueue(familyIndex, 0);
    }

    ~CVulkanQueue() {
        queue.waitIdle();
    }

    void Submit(std::shared_ptr<CVulkanCommandBuffer> commandBuffer, vk::Semaphore submitSemaphore = nullptr, vk::Semaphore waitSemaphore = nullptr, vk::PipelineStageFlags waitSemaphoreFlags = {}, vk::Fence signalFence = nullptr) {
        vk::SubmitInfo submitInfo;
        std::vector<vk::PipelineStageFlags> waitSemaphoreDestinationFlags = {};
        auto vkCommandBuffer = commandBuffer->GetVkCommandBuffer();
        if(waitSemaphore) {
            waitSemaphoreDestinationFlags.push_back(waitSemaphoreFlags);
            submitInfo = vk::SubmitInfo(waitSemaphore, waitSemaphoreDestinationFlags, vkCommandBuffer, submitSemaphore);
        } else {
            submitInfo = vk::SubmitInfo(nullptr, waitSemaphoreDestinationFlags, vkCommandBuffer);
        }

        if(signalFence) {
            queue.submit(submitInfo, signalFence);
        } else { // If we do not have a fence already, create one and wait for it to be signalled.
            auto fence = device.createFenceUnique({});
            queue.submit(submitInfo, *fence);
            vk::Result waitForFencesResult = device.waitForFences(*fence, true, std::numeric_limits<uint64_t>::max());
            if(waitForFencesResult != vk::Result::eSuccess) {
                printf("CVulkanQueue::Submit: Failed to wait for fence");
            }
        }
    }

    CVulkanCommandPool CreateCommandPool(vk::CommandPoolCreateFlags flags = {}) {
        return CVulkanCommandPool(device, familyIndex);
    }

    vk::Queue GetVkQueue() {
        return queue;
    }

    uint32_t GetFamilyIndex() {
        return familyIndex;
    }
};