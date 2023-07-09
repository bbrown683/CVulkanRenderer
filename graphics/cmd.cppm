module;
#include "platform/vulkan.hpp"
export module cmd;
import <optional>;

export class CVulkanCommandPool {
    vk::Device device;
    vk::UniqueCommandPool commandPool;
public:
    CVulkanCommandPool() = default;
    CVulkanCommandPool(vk::Device device, uint8_t queueFamilyIndex) : device(device) {
        commandPool = device.createCommandPoolUnique(vk::CommandPoolCreateInfo({}, queueFamilyIndex));
    }

    void Reset() {
        device.resetCommandPool(*commandPool);
    }

    vk::CommandPool GetVkCommandPool() {
        return *commandPool;
    }
};

export class CVulkanCommandBuffer {
    vk::Device device;
    vk::UniqueCommandBuffer commandBuffer;
    bool active;
    bool rendering;
public:
    CVulkanCommandBuffer() = default;
    CVulkanCommandBuffer(vk::Device device, vk::CommandPool commandPool, vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) : device(device) {
        auto commandBufferInfo = vk::CommandBufferAllocateInfo(commandPool, level, 1);
        commandBuffer = std::move(device.allocateCommandBuffersUnique(commandBufferInfo).front());
    }

    vk::CommandBuffer GetVkCommandBuffer() {
        return *commandBuffer;
    }

    void CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::BufferCopy regions) {
        Begin();

        commandBuffer->copyBuffer(srcBuffer, dstBuffer, regions);

        End();
    }

    void ExecuteCommandBuffers(std::vector<vk::CommandBuffer> commandBuffers) {
        commandBuffer->executeCommands(commandBuffers);
    }

    void BeginRender(vk::Rect2D renderArea, std::vector<vk::RenderingAttachmentInfo> colorAttachments, std::vector<vk::RenderingAttachmentInfo> depthAttachments = {}, std::vector<vk::RenderingAttachmentInfo> stencilAttachments = {}) {
        Begin();
        rendering = true;
        auto renderingInfo = vk::RenderingInfo({}, renderArea, 0, 0, colorAttachments, depthAttachments.data(), stencilAttachments.data());
        commandBuffer->beginRendering(renderingInfo);
    }

    void EndRender() {
        commandBuffer->endRendering();
        End();
        rendering = false;
    }

    // TODO: Support indexed draws.
    void Draw(vk::Pipeline pipeline, vk::PipelineBindPoint bindPoint, uint32_t vertices) {
        if(active && rendering) {
            commandBuffer->draw(vertices, 1, 0, 0);
        } else {
            printf("CVulkanCommandBuffer::Draw: Called outside of CommmandBuffer execution");
        }
    }

private:
    void Begin() {
        device.waitIdle();
        active = true;
        commandBuffer->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    }

    void End() {
        commandBuffer->end();
        active = false;
    }
};