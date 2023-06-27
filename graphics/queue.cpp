module;
#include "platform/vulkan.hpp"
module queue;
import <optional>;
import device;

CVulkanQueue::CVulkanQueue(vk::Device device, uint8_t familyIndex) : device(device), familyIndex(familyIndex) {
    queue = device.getQueue(familyIndex, 0);
    auto semaphoreInfo = vk::SemaphoreCreateInfo(vk::SemaphoreCreateFlags());
    submitSemaphore = device.createSemaphoreUnique(semaphoreInfo);
}

void CVulkanQueue::Submit(vk::CommandBuffer commandBuffer, std::optional<vk::Semaphore> waitSemaphore, std::optional<vk::Fence> signalFence) {
    auto submitInfo = [&, commandBuffer, waitSemaphore]() {
        std::vector<vk::PipelineStageFlags> pipelineStageFlags = {};
        if(waitSemaphore.has_value()) {
            pipelineStageFlags.push_back(vk::PipelineStageFlagBits::eColorAttachmentOutput);
            return vk::SubmitInfo(waitSemaphore.value(), pipelineStageFlags, commandBuffer, *submitSemaphore);
        }
        return vk::SubmitInfo(nullptr, pipelineStageFlags, commandBuffer);
    };
    queue.submit(submitInfo(), signalFence.value_or(nullptr));
}

vk::Queue CVulkanQueue::GetVkQueue() {
    return queue;
}

uint8_t CVulkanQueue::GetFamilyIndex() {
    return familyIndex;
}
