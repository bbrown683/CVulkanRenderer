module;
#include "platform/vulkan.hpp"
#include <FreeImage.h>
export module texture;
import <exception>;

export enum CVulkanTextureCreationError {
    IMAGE_UNSUPPORTED,
    IMAGE_LOAD_FAILED,
    IMAGE_INVALID_MEMORY_TYPE
};

export class CVulkanTextureCreationException : public std::exception {
    CVulkanTextureCreationError error;
public:
    CVulkanTextureCreationException(CVulkanTextureCreationError error) : error(error) {}

    char* what() {
        return (char*)"Failed to create texture due to error: " + error;
    }

    CVulkanTextureCreationError GetError() {
        return error;
    }
};

export class CVulkanTexture {
    vk::UniqueImage image;
    vk::UniqueDeviceMemory imageMemory;
    vk::UniqueImageView imageView;
public:
    CVulkanTexture(vk::Device device, vk::PhysicalDeviceMemoryProperties physicalDeviceMemoryProperties, uint8_t* bits, uint16_t width, uint16_t height, vk::Format format, vk::MemoryPropertyFlagBits memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal, uint8_t mipLevels = 1, vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1) {
        auto extent = vk::Extent3D(width, height);
        auto imageInfo = vk::ImageCreateInfo({}, vk::ImageType::e2D, format,
                                             extent, mipLevels, 1, samples, vk::ImageTiling::eOptimal,
                                             vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                                             vk::SharingMode::eExclusive);
        image = device.createImageUnique(imageInfo);
        vk::MemoryRequirements memoryRequirements = device.getImageMemoryRequirements(*image);
        uint8_t memoryTypeIndex = FindMemoryTypeIndex(memoryRequirements, physicalDeviceMemoryProperties, memoryProperties);
        if(memoryTypeIndex == -1) {
            throw new CVulkanTextureCreationException(CVulkanTextureCreationError::IMAGE_INVALID_MEMORY_TYPE);
        }
        auto allocateInfo = vk::MemoryAllocateInfo(memoryRequirements.size, memoryTypeIndex);
        imageMemory = device.allocateMemoryUnique(allocateInfo);
        device.bindImageMemory(*image, *imageMemory, 0);
    }
    
    CVulkanTexture(vk::Device device, vk::PhysicalDeviceMemoryProperties physicalDeviceMemoryProperties, std::string path, vk::Format format, vk::MemoryPropertyFlagBits memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal, uint8_t mipLevels = 1, vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1) {
        FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(path.c_str());
        if(!FreeImage_FIFSupportsReading(fif)) {
            throw new CVulkanTextureCreationException(CVulkanTextureCreationError::IMAGE_UNSUPPORTED);
        }

        FIBITMAP* bitmap = FreeImage_Load(fif, path.c_str());
        if(!bitmap) {
            throw new CVulkanTextureCreationException(CVulkanTextureCreationError::IMAGE_LOAD_FAILED);
        }

        if(FreeImage_GetBPP(bitmap) != 32) {
            FIBITMAP* temp = bitmap;
            bitmap = FreeImage_ConvertTo32Bits(bitmap);
            FreeImage_Unload(temp);
        }

        FreeImage_FlipVertical(bitmap);

        auto width = static_cast<uint32_t>(FreeImage_GetWidth(bitmap));
        auto height = static_cast<uint32_t>(FreeImage_GetHeight(bitmap));
        uint8_t* bits = static_cast<uint8_t*>(FreeImage_GetBits(bitmap));
        
        CVulkanTexture(device, physicalDeviceMemoryProperties, bits, width, height, format, memoryProperties, mipLevels, samples);
        
        FreeImage_Unload(bitmap);
    }   

private:
    uint8_t FindMemoryTypeIndex(vk::MemoryRequirements memoryRequirements, vk::PhysicalDeviceMemoryProperties physicalDeviceMemoryProperties, vk::MemoryPropertyFlagBits flags) {
        auto memoryTypeBits = memoryRequirements.memoryTypeBits;
        auto memoryTypes = physicalDeviceMemoryProperties.memoryTypes;
        for(int i = 0; i < memoryTypes.size(); i++) {
            if((memoryTypeBits & (1 << i)) && (memoryTypes[i].propertyFlags & flags) == flags) {
                return i;
            }
        }
        return -1;
    }
};