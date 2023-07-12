module;
#include "platform/vulkan.hpp"
export module cmd;
import <optional>;
import swapchain;

export class CVulkanCommandPool {
    vk::Device device;
    vk::UniqueCommandPool commandPool;
public:
    CVulkanCommandPool() = default;
    CVulkanCommandPool(vk::Device device, uint32_t queueFamilyIndex, vk::CommandPoolCreateFlags flags = {}) : device(device) {
        commandPool = device.createCommandPoolUnique(vk::CommandPoolCreateInfo(flags, queueFamilyIndex));
    }

    void Reset() {
        device.waitIdle();
        device.resetCommandPool(*commandPool);
    }

    vk::CommandPool GetVkCommandPool() {
        return *commandPool;
    }
};

// Data that can be passed into a draw call.
export struct CVulkanRender {
    vk::Pipeline pipeline;
    std::vector<vk::RenderingAttachmentInfo> colorAttachments;
    std::vector<vk::RenderingAttachmentInfo> depthAttachments;
    std::vector<vk::RenderingAttachmentInfo> stencilAttachments;
    uint32_t verticesCount;
    std::vector<vk::Buffer> vertexBuffers; 
    std::vector<vk::DeviceSize> vertexBufferOffsets;
    uint32_t indicesCount;
    vk::Buffer indexBuffer;
    vk::DeviceSize indexBufferOffset;
};

export class CVulkanCommandBuffer {
    vk::Device device;
    vk::UniqueCommandBuffer commandBuffer;
public:
    CVulkanCommandBuffer() = default;
    CVulkanCommandBuffer(vk::Device device, vk::CommandPool commandPool, vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) : device(device) {
        auto commandBufferInfo = vk::CommandBufferAllocateInfo(commandPool, level, 1);
        commandBuffer = std::move(device.allocateCommandBuffersUnique(commandBufferInfo).front());
    }

    void Render(CVulkanFrame frame, CVulkanRender render) {
        Begin();

        // Transition image for drawing.
        vk::ImageMemoryBarrier colorImageMemoryBarrier({}, vk::AccessFlagBits::eColorAttachmentWrite, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 0, 0, frame.image, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
        commandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput, {}, nullptr, nullptr, colorImageMemoryBarrier);

        vk::Rect2D renderArea({}, frame.extent);
        vk::RenderingInfo renderingInfo({}, renderArea, 1, 0, render.colorAttachments, render.depthAttachments.data(), render.stencilAttachments.data());
        commandBuffer->beginRendering(renderingInfo);

        vk::Viewport viewport(0.0f, 0.0f, static_cast<float>(frame.extent.width), static_cast<float>(frame.extent.height), 0.0f, 1.0f);
        commandBuffer->setViewport(0, viewport);
        commandBuffer->setScissor(0, renderArea);

        commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, render.pipeline);
        commandBuffer->bindVertexBuffers(0, render.vertexBuffers, render.vertexBufferOffsets);

        if(render.indicesCount > 0) {
            commandBuffer->bindIndexBuffer(render.indexBuffer, render.indexBufferOffset, vk::IndexType::eUint16);
            commandBuffer->drawIndexed(render.indicesCount, 1, 0, 0, 0);
        } else {
            commandBuffer->draw(render.verticesCount, 1, 0, 0);
        }
        
        commandBuffer->endRendering();

        // Transition image for presentation.
        vk::ImageMemoryBarrier presentImageMemoryBarrier(vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eNone, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, 0, 0, frame.image, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
        commandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eBottomOfPipe, {}, nullptr, nullptr, presentImageMemoryBarrier);

        End();
    }

    void CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::BufferCopy regions) {
        Begin();

        commandBuffer->copyBuffer(srcBuffer, dstBuffer, regions);
    
        End();
    }

    void ExecuteCommandBuffers(std::vector<vk::CommandBuffer> commandBuffers) {
        commandBuffer->executeCommands(commandBuffers);
    }
    
    void Reset() {
        commandBuffer->reset();
    }

    vk::CommandBuffer GetVkCommandBuffer() {
        return *commandBuffer;
    }
private:
    void Begin() {
        commandBuffer->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    }

    void End() {
        commandBuffer->end();
    }
};