#include "queue.hpp"

#include "cmd.hpp"

CVulkanQueue::CVulkanQueue(std::shared_ptr<vk::raii::Device> device, uint32_t familyIndex) : device(device), familyIndex(familyIndex) {
    queue = std::make_shared<vk::raii::Queue>(*device, familyIndex, 0);
}

CVulkanQueue::~CVulkanQueue() {
    queue->waitIdle();
}

void CVulkanQueue::Submit(std::shared_ptr<CVulkanCommandBuffer> commandBuffer, vk::Semaphore submitSemaphore,
    vk::Semaphore waitSemaphore, vk::PipelineStageFlags waitSemaphoreFlags,
    vk::Fence signalFence) {
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
        queue->submit(submitInfo, signalFence);
    } else { // If we do not have a fence already, create one and wait for it to be signalled.
        auto fence = vk::raii::Fence(*device, vk::FenceCreateInfo());
        queue->submit(submitInfo, *fence);
        vk::Result waitForFencesResult = device->waitForFences(*fence, true, std::numeric_limits<uint64_t>::max());
        if(waitForFencesResult != vk::Result::eSuccess) {
            printf("CVulkanQueue::Submit: Failed to wait for fence");
        }
    }
}

CVulkanCommandPool CVulkanQueue::CreateCommandPool(vk::CommandPoolCreateFlags flags) {
    return CVulkanCommandPool(device, familyIndex, flags);
}

std::shared_ptr<vk::raii::Queue> CVulkanQueue::GetVkQueue() {
    return queue;
}

uint32_t CVulkanQueue::GetFamilyIndex() {
    return familyIndex;
}
