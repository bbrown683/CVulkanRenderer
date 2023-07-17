module;
#include "platform/vulkan.hpp"
#include <FreeImage.h>
export module image;
import <exception>;

export enum CVulkanImageCreationError {
    IMAGE_UNSUPPORTED,
    IMAGE_LOAD_FAILED,
    IMAGE_INVALID_MEMORY_TYPE
};

export class CVulkanImageCreationException : public std::exception {
    CVulkanImageCreationError error;
public:
    CVulkanImageCreationException(CVulkanImageCreationError error) : error(error) {}

    char* what() {
        return (char*)"Failed to create image due to error: " + error;
    }

    CVulkanImageCreationError GetError() {
        return error;
    }
};

export class CVulkanImage {
    vk::Image swapchainImage;
    vk::UniqueImage image;
    vk::UniqueImageView imageView;
    vk::UniqueDeviceMemory imageMemory; // Used for data thats being written to the image.
    vk::Extent3D extent;
    vk::ImageSubresourceRange range;
public:
    // Takes an existing image.
    CVulkanImage(vk::Device device, vk::Image image, vk::Extent3D extent, vk::Format format) : swapchainImage(image), extent(extent) {
        range = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

        vk::ImageViewCreateInfo imageViewInfo;
        imageViewInfo.setImage(image);
        imageViewInfo.setViewType(vk::ImageViewType::e2D);
        imageViewInfo.setFormat(format);
        imageViewInfo.setSubresourceRange(range);
        imageView = device.createImageViewUnique(imageViewInfo);
    }

    // Creates an image from bits.
    CVulkanImage(vk::Device device, vk::PhysicalDeviceMemoryProperties memoryProperties, vk::Extent3D extent, vk::Format format,
                 vk::MemoryPropertyFlagBits memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal, uint8_t mipLevels = 1, 
                 vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1, uint8_t* bits = nullptr) 
                : extent(extent) {
        range = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
        auto imageInfo = vk::ImageCreateInfo({}, vk::ImageType::e2D, format,
                                             extent, mipLevels, 1, samples, vk::ImageTiling::eOptimal,
                                             vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                                             vk::SharingMode::eExclusive);
        image = device.createImageUnique(imageInfo);

        vk::ImageViewCreateInfo imageViewInfo;
        imageViewInfo.setImage(*image);
        imageViewInfo.setViewType(vk::ImageViewType::e2D);
        imageViewInfo.setFormat(format);
        imageViewInfo.setSubresourceRange(range);
        imageView = device.createImageViewUnique(imageViewInfo);

        if(bits != nullptr) {
            vk::MemoryRequirements memoryRequirements = device.getImageMemoryRequirements(*image);
            uint8_t memoryTypeIndex = FindMemoryTypeIndex(memoryRequirements, memoryProperties, memoryPropertyFlags);
            if(memoryTypeIndex == -1) {
                throw new CVulkanImageCreationException(CVulkanImageCreationError::IMAGE_INVALID_MEMORY_TYPE);
            }
            auto allocateInfo = vk::MemoryAllocateInfo(memoryRequirements.size, memoryTypeIndex);
            imageMemory = device.allocateMemoryUnique(allocateInfo);
            device.bindImageMemory(*image, *imageMemory, 0);
        }
    }
    
    CVulkanImage(vk::Device device, vk::PhysicalDeviceMemoryProperties memoryProperties, std::string path, vk::Format format,
                 vk::MemoryPropertyFlagBits memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal, uint8_t mipLevels = 1,
                 vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1) {
        FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(path.c_str());
        if(!FreeImage_FIFSupportsReading(fif)) {
            throw new CVulkanImageCreationException(CVulkanImageCreationError::IMAGE_UNSUPPORTED);
        }

        FIBITMAP* bitmap = FreeImage_Load(fif, path.c_str());
        if(!bitmap) {
            throw new CVulkanImageCreationException(CVulkanImageCreationError::IMAGE_LOAD_FAILED);
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
        
        CVulkanImage(device, memoryProperties, vk::Extent3D(width, height), format, memoryPropertyFlags, mipLevels, samples, bits);
        
        FreeImage_Unload(bitmap);
    }   

    vk::Image GetVkImage() {
        if(swapchainImage) {
            return swapchainImage;
        }
        return *image;
    }

    vk::ImageView GetVkImageView() {
        return *imageView;
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