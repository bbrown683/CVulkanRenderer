#include "image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "platform/stb/stb_image.h"

#include "device.hpp"
#include "queue.hpp"
#include "cmd.hpp"
#include "buffer.hpp"
#include "util.hpp"

CVulkanImage::CVulkanImage(std::shared_ptr<vk::raii::Device> device, vk::PhysicalDeviceMemoryProperties memoryProperties, vk::Extent3D extent, vk::Format format, uint8_t mipLevels, vk::SampleCountFlagBits samples) {
    auto imageInfo = vk::ImageCreateInfo({}, vk::ImageType::e2D, format,
        extent, mipLevels, 1, samples, vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        vk::SharingMode::eExclusive);
    image = std::make_unique<vk::raii::Image>(*device, imageInfo);

    vk::MemoryRequirements memoryRequirements = image->getMemoryRequirements();
    auto memoryTypeIndex = GetMemoryTypeIndex(memoryProperties, memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
    if(memoryTypeIndex == -1) {
        throw CVulkanImageCreationException(CVulkanImageCreationError::IMAGE_INVALID_MEMORY_TYPE);
    }
    vk::MemoryAllocateInfo allocateInfo(memoryRequirements.size, memoryTypeIndex);
    memory = std::make_unique<vk::raii::DeviceMemory>(*device, allocateInfo);
    image->bindMemory(**memory, 0);
}

vk::Image CVulkanImage::GetVkImage() {
    return **image;
}

CVulkanImageLoader::CVulkanImageLoader(CVulkanDevice* device, CVulkanQueue* transferQueue, std::shared_ptr<CVulkanCommandBuffer> transferCommandBuffer)
    : device(device), transferQueue(transferQueue), transferCommandBuffer(transferCommandBuffer) {}

CVulkanImage CVulkanImageLoader::Load(std::string path, vk::Format format, uint8_t mipLevels, vk::SampleCountFlagBits samples) {
    auto vkDevice = device->GetVkDevice();

    int width, height, channels;
    stbi_uc* bitmap = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if(!bitmap) {
        throw CVulkanImageCreationException(CVulkanImageCreationError::IMAGE_LOAD_FAILED);
    }

    vk::DeviceSize size = width * height * 4;
    auto stagingBuffer = device->CreateBuffer(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, vk::BufferUsageFlagBits::eTransferSrc, bitmap, size);
    stbi_image_free(bitmap);

    vk::Extent3D extent = vk::Extent3D(width, height, 1);
    CVulkanImage image = device->CreateImage(extent, format, mipLevels, samples);
    
    vk::BufferImageCopy region;
    region.bufferOffset = 0;
    region.bufferImageHeight = 0;
    region.bufferRowLength = 0;
    region.imageSubresource = vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
    region.imageOffset = vk::Offset3D { 0, 0, 0 };
    region.imageExtent = extent;

    transferCommandBuffer->CopyBufferToImage(&stagingBuffer, &image, vk::ImageLayout::eTransferDstOptimal, region);
    transferQueue->Submit(transferCommandBuffer);
    transferCommandBuffer->Reset();
    return image;
}
