module;
#include "platform/vulkan.hpp"
#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
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
    CVulkanBufferAllocator() = default;
    CVulkanBufferAllocator(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device) {
        VmaVulkanFunctions functions {};
        functions.vkAllocateMemory = VULKAN_HPP_DEFAULT_DISPATCHER.vkAllocateMemory;
        functions.vkFreeMemory = VULKAN_HPP_DEFAULT_DISPATCHER.vkFreeMemory;
        functions.vkBindBufferMemory = VULKAN_HPP_DEFAULT_DISPATCHER.vkBindBufferMemory;
        functions.vkBindBufferMemory2KHR = VULKAN_HPP_DEFAULT_DISPATCHER.vkBindBufferMemory2;
        functions.vkBindImageMemory = VULKAN_HPP_DEFAULT_DISPATCHER.vkBindImageMemory;
        functions.vkBindImageMemory2KHR = VULKAN_HPP_DEFAULT_DISPATCHER.vkBindImageMemory2;
        functions.vkCmdCopyBuffer = VULKAN_HPP_DEFAULT_DISPATCHER.vkCmdCopyBuffer;
        functions.vkMapMemory = VULKAN_HPP_DEFAULT_DISPATCHER.vkMapMemory;
        functions.vkUnmapMemory = VULKAN_HPP_DEFAULT_DISPATCHER.vkUnmapMemory;
        functions.vkCreateBuffer = VULKAN_HPP_DEFAULT_DISPATCHER.vkCreateBuffer;
        functions.vkDestroyBuffer = VULKAN_HPP_DEFAULT_DISPATCHER.vkDestroyBuffer;
        functions.vkCreateImage = VULKAN_HPP_DEFAULT_DISPATCHER.vkCreateImage;
        functions.vkDestroyImage = VULKAN_HPP_DEFAULT_DISPATCHER.vkDestroyImage;
        functions.vkGetPhysicalDeviceProperties = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetPhysicalDeviceProperties;
        functions.vkGetPhysicalDeviceMemoryProperties = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetPhysicalDeviceMemoryProperties;
        functions.vkGetPhysicalDeviceMemoryProperties2KHR = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetPhysicalDeviceMemoryProperties2;
        functions.vkGetBufferMemoryRequirements = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetBufferMemoryRequirements;
        functions.vkGetBufferMemoryRequirements2KHR = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetBufferMemoryRequirements2;
        functions.vkGetDeviceBufferMemoryRequirements = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceBufferMemoryRequirements;
        functions.vkGetImageMemoryRequirements = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetImageMemoryRequirements;
        functions.vkGetImageMemoryRequirements2KHR = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetImageMemoryRequirements2;
        functions.vkGetDeviceImageMemoryRequirements = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceImageMemoryRequirements;
        functions.vkFlushMappedMemoryRanges = VULKAN_HPP_DEFAULT_DISPATCHER.vkFlushMappedMemoryRanges;
        functions.vkInvalidateMappedMemoryRanges = VULKAN_HPP_DEFAULT_DISPATCHER.vkInvalidateMappedMemoryRanges;

        VmaAllocatorCreateInfo allocatorCreateInfo = {};
        allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
        allocatorCreateInfo.physicalDevice = physicalDevice;
        allocatorCreateInfo.device = device;
        allocatorCreateInfo.instance = instance;
        allocatorCreateInfo.pVulkanFunctions = &functions;
        vk::Result result = vk::Result(vmaCreateAllocator(&allocatorCreateInfo, &allocator));
        if(result != vk::Result::eSuccess) {
            throw CVulkanBufferAllocatorCreationException(EBufferAllocatorCreationError::ALLOCATOR_CREATION_FAILED);
        }
    }
    VmaAllocation CreateBuffer() {
        // TODO: Fill these out.
        VmaAllocation allocation;
        vk::MemoryRequirements memoryRequirements;

        VmaAllocationCreateInfo allocationCreateInfo;
        VmaAllocationInfo allocationInfo;
        vk::Result result = vk::Result(vmaAllocateMemory(allocator, &memoryRequirements.vk::MemoryRequirements::operator const VkMemoryRequirements & (), &allocationCreateInfo, &allocation, &allocationInfo));
        if(result == vk::Result::eSuccess) {
            return allocation;
        }
        return nullptr;
    }

    void FreeBuffer(VmaAllocation allocation) {
        vmaFreeMemory(allocator, allocation);
    }

    VmaAllocator* GetVmaAllocator() {
        return &allocator;
    }
};

export class CVulkanBuffer {
    vk::UniqueBuffer buffer;
    vk::Device device;
    VmaAllocator* allocator;
    VmaAllocation allocation;
public:
    CVulkanBuffer() = default;
    CVulkanBuffer(vk::Device device, VmaAllocator* allocator) : device(device), allocator(allocator) {

    }

    ~CVulkanBuffer() {

    }
};