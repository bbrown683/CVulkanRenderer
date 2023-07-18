#include "buffer.hpp"

#include "util.hpp"

CVulkanBuffer::CVulkanBuffer(std::shared_ptr<vk::raii::Device> device, vk::PhysicalDeviceMemoryProperties memoryProperties, vk::MemoryPropertyFlags desiredPropertyFlags, 
    vk::BufferUsageFlags usage, void* data, vk::DeviceSize dataSize) {
    auto bufferInfo = vk::BufferCreateInfo({}, dataSize, usage);
    buffer = std::make_unique<vk::raii::Buffer>(*device, bufferInfo);

    vk::MemoryRequirements memoryRequirements = buffer->getMemoryRequirements();
    auto memoryTypeIndex = GetMemoryTypeIndex(memoryProperties, memoryRequirements.memoryTypeBits, desiredPropertyFlags);
    if (memoryTypeIndex == -1) {
        throw CVulkanBufferCreationException(CVulkanBufferCreationError::BUFFER_INVALID_MEMORY_TYPE);
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

vk::DeviceSize CVulkanBuffer::GetVkDeviceSize() {
    return size;
}
