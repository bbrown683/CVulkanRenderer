module;
#include "platform/vulkan.hpp"
#include <vma/vk_mem_alloc.h>
export module buffer;
import <exception>;

export enum EBufferAllocatorCreationError {
    ALLOCATOR_CREATION_FAILED,
};

export class CVulkanBufferAllocatorCreationException : public std::exception {
    EBufferAllocatorCreationError error;
public:
    CVulkanBufferAllocatorCreationException(EBufferAllocatorCreationError error) : error(error) {}

    char* what() {
        return (char*)"Failed to create buffer allocator due to error: " + error;
    }

    EBufferAllocatorCreationError GetError() {
        return error;
    }
};

export class CVulkanBufferAllocator {
    VmaAllocator allocator;
public:
    CVulkanBufferAllocator(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device);
    VmaAllocation CreateBuffer();
    void FreeBuffer(VmaAllocation allocation);
    VmaAllocator* GetVmaAllocator();
};

export class CVulkanBuffer {
    vk::UniqueBuffer buffer;
    vk::Device device;
    VmaAllocator* allocator;
    VmaAllocation allocation;
public:
    CVulkanBuffer(vk::Device device, VmaAllocator* allocator);
    ~CVulkanBuffer();
};