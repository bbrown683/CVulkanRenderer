module;
#include "platform/vulkan.hpp"
#include "platform/imgui/imgui_impl_vulkan.h"
export module cmd;
import <optional>;
import buffer;
import types;

export class CVulkanCommandBuffer {
    vk::Device device;
    vk::UniqueCommandBuffer commandBuffer;
public:
    CVulkanCommandBuffer(vk::Device device, vk::CommandPool commandPool, vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) : device(device) {
        auto commandBufferInfo = vk::CommandBufferAllocateInfo(commandPool, level, 1);
        commandBuffer = std::move(device.allocateCommandBuffersUnique(commandBufferInfo).front());
    }

    void Render(CVulkanFrame frame, CVulkanRender render) {
        Begin();

        // Transition image for drawing.
        vk::ImageMemoryBarrier colorAttachmentBarrier;
        colorAttachmentBarrier.setImage(frame.image);
        colorAttachmentBarrier.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
        colorAttachmentBarrier.setOldLayout(vk::ImageLayout::eUndefined);
        colorAttachmentBarrier.setNewLayout(vk::ImageLayout::eColorAttachmentOptimal);
        colorAttachmentBarrier.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

        commandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput, {}, nullptr, nullptr, colorAttachmentBarrier);

        vk::Rect2D renderArea({}, frame.extent);
        vk::RenderingInfo renderingInfo;
        renderingInfo.setRenderArea(renderArea);
        renderingInfo.setLayerCount(1);
        renderingInfo.setColorAttachments(render.colorAttachments);
        renderingInfo.setPDepthAttachment(render.depthAttachments.data());
        renderingInfo.setPStencilAttachment(render.stencilAttachments.data());

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
        vk::ImageMemoryBarrier presentImageBarrier;
        presentImageBarrier.setImage(frame.image);
        presentImageBarrier.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
        presentImageBarrier.setOldLayout(vk::ImageLayout::eColorAttachmentOptimal);
        presentImageBarrier.setNewLayout(vk::ImageLayout::ePresentSrcKHR);
        presentImageBarrier.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

        commandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eBottomOfPipe, {}, nullptr, nullptr, presentImageBarrier);

        End();
    }

    void CopyBuffer(CVulkanBuffer* srcBuffer, CVulkanBuffer* dstBuffer, vk::BufferCopy regions) {
        Begin();
        commandBuffer->copyBuffer(srcBuffer->GetVkBuffer(), dstBuffer->GetVkBuffer(), regions);
        End();
    }

    void ExecuteCommandBuffers(std::vector<vk::CommandBuffer> commandBuffers) {
        commandBuffer->executeCommands(commandBuffers);
    }

    void UploadImguiFonts() {
        Begin();
        ImGui_ImplVulkan_CreateFontsTexture(*commandBuffer);
        End();
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

export class CVulkanCommandPool {
    vk::Device device;
    vk::UniqueCommandPool commandPool;
public:
    CVulkanCommandPool(vk::Device device, uint32_t queueFamilyIndex, vk::CommandPoolCreateFlags flags = {}) : device(device) {
        commandPool = device.createCommandPoolUnique(vk::CommandPoolCreateInfo(flags, queueFamilyIndex));
    }

    void Reset() {
        device.waitIdle();
        device.resetCommandPool(*commandPool);
    }

    CVulkanCommandBuffer CreateCommandBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) {
        return CVulkanCommandBuffer(device, *commandPool, level);
    }

    vk::CommandPool GetVkCommandPool() {
        return *commandPool;
    }
};