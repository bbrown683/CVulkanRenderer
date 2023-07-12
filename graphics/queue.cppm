module;
#include "platform/vulkan.hpp"
export module queue;
import device;

export class CVulkanQueue {
    std::shared_ptr<CVulkanDevice> device;
    vk::Queue queue;
    uint32_t familyIndex;
    std::vector<vk::UniqueSemaphore> submitSemaphores;
public:
    CVulkanQueue() = default;
    CVulkanQueue(std::shared_ptr<CVulkanDevice> device, uint32_t familyIndex) : device(device), familyIndex(familyIndex) {
        queue = device->GetVkDevice().getQueue(familyIndex, 0);
    }

    ~CVulkanQueue() {
        queue.waitIdle();
    }

    void Submit(vk::CommandBuffer commandBuffer, vk::Semaphore submitSemaphore = nullptr, vk::Semaphore waitSemaphore = nullptr, vk::PipelineStageFlags waitSemaphoreFlags = {}, vk::Fence signalFence = nullptr) {
        vk::SubmitInfo submitInfo;
        std::vector<vk::PipelineStageFlags> waitSemaphoreDestinationFlags = {};
        if(waitSemaphore) {
            waitSemaphoreDestinationFlags.push_back(waitSemaphoreFlags);
            submitInfo = vk::SubmitInfo(waitSemaphore, waitSemaphoreDestinationFlags, commandBuffer, submitSemaphore);
        } else {
            submitInfo = vk::SubmitInfo(nullptr, waitSemaphoreDestinationFlags, commandBuffer);
        }

        if(signalFence) {
            queue.submit(submitInfo, signalFence);
        } else { // If we do not have a fence already, create one and wait for it to be signalled.
            auto fence = device->GetVkDevice().createFenceUnique({});
            queue.submit(submitInfo, *fence);
            vk::Result waitForFencesResult = device->GetVkDevice().waitForFences(*fence, true, std::numeric_limits<uint64_t>::max());
            if(waitForFencesResult != vk::Result::eSuccess) {
                printf("CVulkanQueue::Submit: Failed to wait for fence");
            }
        }
    }

    vk::Queue GetVkQueue() {
        return queue;
    }

    uint32_t GetFamilyIndex() {
        return familyIndex;
    }
};