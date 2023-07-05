module;
#include "platform/vulkan.hpp"
export module queue;

export class CVulkanQueue {
    vk::Queue queue;
    uint8_t familyIndex;
    std::vector<vk::UniqueSemaphore> submitSemaphores;
public:
    CVulkanQueue() = default;
    CVulkanQueue(vk::Device device, uint8_t familyIndex, uint8_t semaphoreCount = 1) : familyIndex(familyIndex) {
        queue = device.getQueue(familyIndex, 0);
        auto semaphoreInfo = vk::SemaphoreCreateInfo({});
        for(uint8_t i = 0; i < semaphoreCount; i++) {
            submitSemaphores.push_back(device.createSemaphoreUnique(semaphoreInfo));
        }
    }

    void Submit(vk::CommandBuffer commandBuffer, uint8_t currentSemaphore = 0, vk::Optional<vk::Semaphore> waitSemaphore = nullptr, vk::Optional<vk::Fence> signalFence = nullptr) {
        auto submitInfo = [&, commandBuffer, waitSemaphore]() {
            std::vector<vk::PipelineStageFlags> pipelineStageFlags = {};
            if(waitSemaphore != nullptr) {
                pipelineStageFlags.push_back(vk::PipelineStageFlagBits::eColorAttachmentOutput);
                return vk::SubmitInfo(*waitSemaphore, pipelineStageFlags, commandBuffer, *submitSemaphores[currentSemaphore]);
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