#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <FreeImage.h>
#include <exception>

enum CVulkanImageCreationError {
    IMAGE_UNSUPPORTED,
    IMAGE_LOAD_FAILED,
    IMAGE_INVALID_MEMORY_TYPE
};

class CVulkanImageCreationException : public std::exception {
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

class CVulkanImage {
    vk::Image swapchainImage;
    std::shared_ptr<vk::raii::Image> image;
    std::shared_ptr<vk::raii::ImageView> view;
    std::shared_ptr<vk::raii::DeviceMemory> memory; // Used for data thats being written to the image.
    vk::Extent3D extent;
    vk::ImageSubresourceRange range;
public:
    // Takes an existing image.
    CVulkanImage(std::shared_ptr<vk::raii::Device> device, vk::Image image, vk::Extent3D extent, vk::Format format) : swapchainImage(image), extent(extent) {
        range = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

        vk::ImageViewCreateInfo imageViewInfo;
        imageViewInfo.setImage(image);
        imageViewInfo.setViewType(vk::ImageViewType::e2D);
        imageViewInfo.setFormat(format);
        imageViewInfo.setSubresourceRange(range);
        view = std::make_unique<vk::raii::ImageView>(*device, imageViewInfo);
    }

    // Creates an image from bits.
    CVulkanImage(std::shared_ptr<vk::raii::Device> device, vk::PhysicalDeviceMemoryProperties memoryProperties, vk::Extent3D extent, vk::Format format,
                 vk::MemoryPropertyFlagBits memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal, uint8_t mipLevels = 1, 
                 vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1, uint8_t* bits = nullptr) 
                : extent(extent) {
        range = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
        auto imageInfo = vk::ImageCreateInfo({}, vk::ImageType::e2D, format,
                                             extent, mipLevels, 1, samples, vk::ImageTiling::eOptimal,
                                             vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                                             vk::SharingMode::eExclusive);
        image = std::make_unique<vk::raii::Image>(*device, imageInfo);

        vk::ImageViewCreateInfo imageViewInfo;
        imageViewInfo.setImage(**image);
        imageViewInfo.setViewType(vk::ImageViewType::e2D);
        imageViewInfo.setFormat(format);
        imageViewInfo.setSubresourceRange(range);
        view = std::make_unique<vk::raii::ImageView>(*device, imageViewInfo);

        if(bits != nullptr) {
            vk::MemoryRequirements memoryRequirements = image->getMemoryRequirements();
            uint8_t memoryTypeIndex = FindMemoryTypeIndex(memoryRequirements, memoryProperties, memoryPropertyFlags);
            if(memoryTypeIndex == -1) {
                throw new CVulkanImageCreationException(CVulkanImageCreationError::IMAGE_INVALID_MEMORY_TYPE);
            }
            auto allocateInfo = vk::MemoryAllocateInfo(memoryRequirements.size, memoryTypeIndex);
            memory = std::make_unique<vk::raii::DeviceMemory>(*device, allocateInfo);
            image->bindMemory(**memory, 0);
        }
    }
    
    CVulkanImage(std::shared_ptr<vk::raii::Device> device, vk::PhysicalDeviceMemoryProperties memoryProperties, std::string path, vk::Format format,
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
        return **image;
    }

    vk::ImageView GetVkImageView() {
        return **view;
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