module;
#include "platform/vulkan.hpp"
export module buffer;
import <exception>;

export class CVulkanBuffer {
    vk::UniqueBuffer buffer;
    vk::UniqueDeviceMemory memory;
public:
    CVulkanBuffer() = default;
    CVulkanBuffer(vk::Device device, vk::PhysicalDeviceMemoryProperties memoryProperties, vk::MemoryPropertyFlags desiredPropertyFlags, vk::BufferUsageFlags usage, void* data, vk::DeviceSize dataSize) {
        auto bufferInfo = vk::BufferCreateInfo({}, dataSize, usage);
        buffer = device.createBufferUnique(bufferInfo);

        vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(*buffer);
        auto memoryTypeIndex = GetMemoryTypeIndex(memoryProperties, memoryRequirements.memoryTypeBits, desiredPropertyFlags);
        if(memoryTypeIndex == -1) {
            printf("CVulkanBuffer::CVulkanBuffer: Failed to find a valid memory type for buffer");
        }
        vk::MemoryAllocateInfo allocateInfo(memoryRequirements.size, memoryTypeIndex);
        memory = device.allocateMemoryUnique(allocateInfo);
        device.bindBufferMemory(*buffer, *memory, 0);

        if(data != nullptr) {
            void* handle = device.mapMemory(*memory, 0, dataSize);
            memcpy(handle, data, static_cast<size_t>(dataSize));
            device.unmapMemory(*memory);
        }
    }

    vk::Buffer GetVkBuffer() {
        return *buffer;
    }

private:
    uint32_t GetMemoryTypeIndex(vk::PhysicalDeviceMemoryProperties memoryProperties, uint32_t memoryTypeBits, vk::MemoryPropertyFlags desiredPropertyFlags) {
        for(uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
            if((memoryTypeBits & 1) == 1) {
                if((memoryProperties.memoryTypes[i].propertyFlags & desiredPropertyFlags) == desiredPropertyFlags) {
                    return i;
                }
            }
            memoryTypeBits >>= 1;
        }
        return -1;
    }
};