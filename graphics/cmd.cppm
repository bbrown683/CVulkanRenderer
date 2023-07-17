module;
#include "platform/vulkan.hpp"
#include "platform/imgui/imgui_impl_vulkan.h"
export module cmd;
import <vector>;
import buffer;
import image;
import types;

export class CVulkanCommandBuffer {
    vk::Device device;
    vk::UniqueCommandBuffer commandBuffer;
public:
    CVulkanCommandBuffer(vk::Device device, vk::CommandPool commandPool, vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) : device(device) {
        auto commandBufferInfo = vk::CommandBufferAllocateInfo(commandPool, level, 1);
        commandBuffer = std::move(device.allocateCommandBuffersUnique(commandBufferInfo).front());
    }

    void TransitionImageLayout(vk::Image image, vk::AccessFlags srcAccessFlags, vk::AccessFlags dstAccessFlags, 
                                  vk::ImageLayout oldLayout, vk::ImageLayout newLayout, 
                                  vk::PipelineStageFlags srcStage, vk::PipelineStageFlags dstStage) {
        // Transition image for drawing.
        vk::ImageMemoryBarrier pipelineBarrier;
        pipelineBarrier.setImage(image);
        pipelineBarrier.setSrcAccessMask(srcAccessFlags);
        pipelineBarrier.setDstAccessMask(dstAccessFlags);
        pipelineBarrier.setOldLayout(oldLayout);
        pipelineBarrier.setNewLayout(newLayout);
        pipelineBarrier.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

        commandBuffer->pipelineBarrier(srcStage, dstStage, {}, nullptr, nullptr, pipelineBarrier);
    }

    void BeginPass(CVulkanFrame* frame, CVulkanRender* render) {
        Begin();
        TransitionImageLayout(frame->image, {}, vk::AccessFlagBits::eColorAttachmentWrite, 
                              vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 
                              vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput);

        vk::Rect2D renderArea({}, frame->extent);
        vk::RenderingInfo renderingInfo;
        renderingInfo.setRenderArea(renderArea);
        renderingInfo.setLayerCount(1);
        renderingInfo.setColorAttachments(render->colorAttachments);
        renderingInfo.setPDepthAttachment(render->depthAttachments.data());
        renderingInfo.setPStencilAttachment(render->stencilAttachments.data());

        commandBuffer->beginRendering(renderingInfo);

        vk::Viewport viewport(0.0f, 0.0f, static_cast<float>(frame->extent.width), static_cast<float>(frame->extent.height), 0.0f, 1.0f);
        commandBuffer->setViewport(0, viewport);
        commandBuffer->setScissor(0, renderArea);
    }

    void EndPass(CVulkanFrame* frame) {
        commandBuffer->endRendering();
        TransitionImageLayout(frame->image, vk::AccessFlagBits::eColorAttachmentWrite, {}, 
                              vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, 
                              vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eBottomOfPipe);
        End();
    }

    void Draw(CVulkanDraw* draw) {
        commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, draw->pipeline);
        commandBuffer->bindVertexBuffers(0, draw->vertexBuffers, draw->vertexBufferOffsets);
        if(draw->indicesCount > 0) {
            commandBuffer->bindIndexBuffer(draw->indexBuffer, draw->indexBufferOffset, vk::IndexType::eUint16);
            commandBuffer->drawIndexed(draw->indicesCount, 1, 0, 0, 0);
        } else {
            commandBuffer->draw(draw->verticesCount, 1, 0, 0);
        }
    }

    void Draw(ImDrawData* drawData) {
        ImGui_ImplVulkan_RenderDrawData(drawData, *commandBuffer);
    }

    void CopyBuffer(CVulkanBuffer* srcBuffer, CVulkanBuffer* dstBuffer, vk::BufferCopy regions) {
        Begin();
        commandBuffer->copyBuffer(srcBuffer->GetVkBuffer(), dstBuffer->GetVkBuffer(), regions);
        End();
    }

    void CopyImage(CVulkanImage* srcImage, CVulkanImage* dstImage, vk::ImageCopy regions) {
        Begin();
        commandBuffer->copyImage(srcImage->GetVkImage(), vk::ImageLayout::eColorAttachmentOptimal, dstImage->GetVkImage(), vk::ImageLayout::eColorAttachmentOptimal, regions);
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