#include "buffer.hpp"

CVulkanBuffer::CVulkanBuffer(std::shared_ptr<vk::raii::Device> device, vk::PhysicalDeviceMemoryProperties memoryProperties, vk::MemoryPropertyFlags desiredPropertyFlags, vk::BufferUsageFlags usage, void* data, vk::DeviceSize dataSize) {
    auto bufferInfo = vk::BufferCreateInfo({}, dataSize, usage);
    buffer = std::make_unique<vk::raii::Buffer>(*device, bufferInfo);

    vk::MemoryRequirements memoryRequirements = buffer->getMemoryRequirements();
    auto memoryTypeIndex = GetMemoryTypeIndex(memoryProperties, memoryRequirements.memoryTypeBits, desiredPropertyFlags);
    if (memoryTypeIndex == -1) {
        printf("CVulkanBuffer::CVulkanBuffer: Failed to find a valid memory type for buffer");
    }
    vk::MemoryAllocateInfo allocateInfo(memoryRequirements.size, memoryTypeIndex);
    memory = std::make_unique<vk::raii::DeviceMemory>(*device, allocateInfo);
    buffer->bindMemory(**memory, 0);

    if (data != nullptr) {
        void* handle = memory->mapMemory(0, dataSize);
        memcpy(handle, data, static_cast<size_t>(dataSize));
        memory->unmapMemory();
    }
}

vk::Buffer CVulkanBuffer::GetVkBuffer() {
    return **buffer;
}

uint32_t CVulkanBuffer::GetMemoryTypeIndex(vk::PhysicalDeviceMemoryProperties memoryProperties, uint32_t memoryTypeBits, vk::MemoryPropertyFlags desiredPropertyFlags) {
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if ((memoryTypeBits & 1) == 1) {
            if ((memoryProperties.memoryTypes[i].propertyFlags & desiredPropertyFlags) == desiredPropertyFlags) {
                return i;
            }
        }
        memoryTypeBits >>= 1;
    }
    return -1;
}
