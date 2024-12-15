#include "cmd.hpp"

#include <imgui/imgui_impl_vulkan.h>
#include "types.hpp"
#include "buffer.hpp"
#include "image.hpp"

CVulkanCommandBuffer::CVulkanCommandBuffer(std::shared_ptr<vk::raii::Device> device, std::shared_ptr<vk::raii::CommandPool> commandPool, vk::CommandBufferLevel level) {
    auto commandBufferInfo = vk::CommandBufferAllocateInfo(**commandPool, level, 1);
    commandBuffer = std::make_unique<vk::raii::CommandBuffer>(std::move(vk::raii::CommandBuffers(*device, commandBufferInfo).front()));
}

void CVulkanCommandBuffer::TransitionImageLayout(vk::Image image, vk::AccessFlags srcAccessFlags, vk::AccessFlags dstAccessFlags, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::PipelineStageFlags srcStage, vk::PipelineStageFlags dstStage) {
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

void CVulkanCommandBuffer::BeginPass(CVulkanFrame* frame, CVulkanRender* render) {
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

void CVulkanCommandBuffer::EndPass(CVulkanFrame* frame) {
    commandBuffer->endRendering();
    TransitionImageLayout(frame->image, vk::AccessFlagBits::eColorAttachmentWrite, {},
        vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
        vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eBottomOfPipe);
    End();
}

void CVulkanCommandBuffer::Draw(CVulkanDraw* draw) {
    commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, draw->pipeline);
    commandBuffer->bindVertexBuffers(0, draw->vertexBuffers, draw->vertexBufferOffsets);
    if(draw->indicesCount > 0) {
        commandBuffer->bindIndexBuffer(draw->indexBuffer, draw->indexBufferOffset, vk::IndexType::eUint16);
        commandBuffer->drawIndexed(draw->indicesCount, 1, 0, 0, 0);
    } else {
        commandBuffer->draw(draw->verticesCount, 1, 0, 0);
    }
}

void CVulkanCommandBuffer::Draw(ImDrawData* drawData) {
    ImGui_ImplVulkan_RenderDrawData(drawData, **commandBuffer);
}

void CVulkanCommandBuffer::CopyBuffer(CVulkanBuffer* srcBuffer, CVulkanBuffer* dstBuffer, vk::BufferCopy regions) {
    Begin();
    commandBuffer->copyBuffer(srcBuffer->GetVkBuffer(), dstBuffer->GetVkBuffer(), regions);
    End();
}

void CVulkanCommandBuffer::CopyImage(CVulkanImage* srcImage, CVulkanImage* dstImage, vk::ImageCopy regions) {
    Begin();
    commandBuffer->copyImage(srcImage->GetVkImage(), vk::ImageLayout::eTransferSrcOptimal, dstImage->GetVkImage(), vk::ImageLayout::eTransferDstOptimal, regions);
    End();
}

void CVulkanCommandBuffer::CopyBufferToImage(CVulkanBuffer* buffer, CVulkanImage* image, vk::ImageLayout layout, vk::BufferImageCopy regions) {
    Begin();
    commandBuffer->copyBufferToImage(buffer->GetVkBuffer(), image->GetVkImage(), layout, regions);
    End();
}

void CVulkanCommandBuffer::ExecuteCommandBuffers(std::vector<std::shared_ptr<CVulkanCommandBuffer>> commandBuffers) {

}

void CVulkanCommandBuffer::UploadImguiFonts() {
    Begin();
    ImGui_ImplVulkan_CreateFontsTexture();
    End();
}

void CVulkanCommandBuffer::Reset() {
    commandBuffer->reset();
}

vk::CommandBuffer CVulkanCommandBuffer::GetVkCommandBuffer() {
    return **commandBuffer;
}

void CVulkanCommandBuffer::Begin() {
    commandBuffer->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
}

void CVulkanCommandBuffer::End() {
    commandBuffer->end();
}

CVulkanCommandPool::CVulkanCommandPool(std::shared_ptr<vk::raii::Device> device, uint32_t queueFamilyIndex, vk::CommandPoolCreateFlags flags) : device(device) {
    commandPool = std::make_shared<vk::raii::CommandPool>(*device, vk::CommandPoolCreateInfo(flags, queueFamilyIndex));
}

void CVulkanCommandPool::Reset() {
    device->waitIdle();
    commandPool->reset();
}

CVulkanCommandBuffer CVulkanCommandPool::CreateCommandBuffer(vk::CommandBufferLevel level) {
    return CVulkanCommandBuffer(device, commandPool, level);
}

vk::CommandPool CVulkanCommandPool::GetVkCommandPool() {
    return **commandPool;
}
