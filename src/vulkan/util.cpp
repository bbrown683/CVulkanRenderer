#include "util.hpp"

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
