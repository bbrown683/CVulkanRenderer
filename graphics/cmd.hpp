#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

struct ImDrawData;
class CVulkanBuffer;
class CVulkanImage;
struct CVulkanDraw;
struct CVulkanFrame;
struct CVulkanRender;

class CVulkanCommandBuffer {
    std::unique_ptr<vk::raii::CommandBuffer> commandBuffer;
public:
    CVulkanCommandBuffer(std::shared_ptr<vk::raii::Device> device, std::shared_ptr<vk::raii::CommandPool> commandPool, vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);
    void TransitionImageLayout(vk::Image image, vk::AccessFlags srcAccessFlags, vk::AccessFlags dstAccessFlags,
        vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
        vk::PipelineStageFlags srcStage, vk::PipelineStageFlags dstStage);
    void BeginPass(CVulkanFrame* frame, CVulkanRender* render);
    void EndPass(CVulkanFrame* frame);
    void Draw(CVulkanDraw* draw);
    void Draw(ImDrawData* drawData);
    void CopyBuffer(CVulkanBuffer* srcBuffer, CVulkanBuffer* dstBuffer, vk::BufferCopy regions);
    void CopyImage(CVulkanImage* srcImage, CVulkanImage* dstImage, vk::ImageCopy regions);
    void CopyBufferToImage(CVulkanBuffer* buffer, CVulkanImage* image, vk::ImageLayout layout, vk::BufferImageCopy regions);
    void ExecuteCommandBuffers(std::vector<std::shared_ptr<CVulkanCommandBuffer>> commandBuffers);
    void UploadImguiFonts();
    vk::CommandBuffer GetVkCommandBuffer();
private:
    void Begin();
    void End();
};

class CVulkanCommandPool {
    std::shared_ptr<vk::raii::Device> device;
    std::shared_ptr<vk::raii::CommandPool> commandPool;
public:
    CVulkanCommandPool(std::shared_ptr<vk::raii::Device> device, uint32_t queueFamilyIndex, vk::CommandPoolCreateFlags flags = {});
    void Reset();
    CVulkanCommandBuffer CreateCommandBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);
    vk::CommandPool GetVkCommandPool();
};