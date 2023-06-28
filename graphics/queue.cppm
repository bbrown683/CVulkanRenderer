module;
#include "platform/vulkan.hpp"
export module queue;

export class CVulkanQueue {
    vk::Device device;
    vk::Queue queue;
    uint8_t familyIndex;
    vk::UniqueSemaphore submitSemaphore;
public:
    CVulkanQueue() = default;
    CVulkanQueue(vk::Device device, uint8_t familyIndex) : device(device), familyIndex(familyIndex) {
        queue = device.getQueue(familyIndex, 0);
        auto semaphoreInfo = vk::SemaphoreCreateInfo(vk::SemaphoreCreateFlags());
        submitSemaphore = device.createSemaphoreUnique(semaphoreInfo);
    }

    void Submit(vk::CommandBuffer commandBuffer, vk::Optional<vk::Semaphore> waitSemaphore = nullptr, vk::Optional<vk::Fence> signalFence = nullptr) {
        auto submitInfo = [&, commandBuffer, waitSemaphore]() {
            std::vector<vk::PipelineStageFlags> pipelineStageFlags = {};
            if(waitSemaphore != nullptr) {
                pipelineStageFlags.push_back(vk::PipelineStageFlagBits::eColorAttachmentOutput);
                return vk::SubmitInfo(*waitSemaphore, pipelineStageFlags, commandBuffer, *submitSemaphore);
            }
            return vk::SubmitInfo(nullptr, pipelineStageFlags, commandBuffer);
        };
        queue.submit(submitInfo(), *signalFence);
    }

    vk::Queue GetVkQueue() {
        return queue;
    }

    uint8_t GetFamilyIndex() {
        return familyIndex;
    }
};