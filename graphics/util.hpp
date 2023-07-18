#pragma once
#include <vulkan/vulkan.hpp>

uint32_t GetMemoryTypeIndex(vk::PhysicalDeviceMemoryProperties memoryProperties, uint32_t memoryTypeBits, vk::MemoryPropertyFlags desiredPropertyFlags);