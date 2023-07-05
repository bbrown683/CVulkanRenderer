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
        commandPool = device.createCommandPoolUnique(vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueFamilyIndex));
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
    bool activeRenderPass;
public:
    CVulkanCommandBuffer() = default;
    CVulkanCommandBuffer(vk::Device device, vk::CommandPool commandPool) : device(device) {
        auto commandBufferInfo = vk::CommandBufferAllocateInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1);
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

    void BeginRenderPass(vk::RenderPass renderPass, vk::Framebuffer framebuffer, vk::Extent2D extent, vk::ClearColorValue clearColor) {
        Begin();

        auto clearValue = vk::ClearValue(clearColor);

        activeRenderPass = true;
        commandBuffer->beginRenderPass(vk::RenderPassBeginInfo(renderPass, framebuffer, vk::Rect2D({ 0, 0 }, extent), clearValue), vk::SubpassContents::eInline);
    }

    void EndRenderPass(vk::RenderPass renderPass, vk::Framebuffer framebuffer, vk::Extent2D extent, vk::ClearColorValue clearColor) {
        activeRenderPass = false;
        commandBuffer->endRenderPass();

        End();
    }

    // TODO: Support indexed draws.
    void Draw(vk::Pipeline pipeline, vk::PipelineBindPoint bindPoint, uint32_t vertices) {
        if(active && activeRenderPass) {
            commandBuffer->draw(vertices, 1, 0, 0);
        } else {
            printf("CVulkanCommandBuffer::Draw: Called outside of CommmandBuffer execution.");
        }
    }

private:
    void Begin() {
        device.waitIdle();
        commandBuffer->reset();

        active = true;
        commandBuffer->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    }

    void End() {
        commandBuffer->end();
        active = false;
    }
};