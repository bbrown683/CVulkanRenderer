#include "swapchain.hpp"

// Enable the WSI extensions
#if defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(__linux__)
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "instance.hpp"
#include "device.hpp"
#include "queue.hpp"
#include "types.hpp"

CVulkanSwapchain::CVulkanSwapchain(CVulkanInstance* pInstance, CVulkanDevice* pDevice, CVulkanQueue* pQueue, SDL_Window* pWindow, uint32_t imageCount, bool vsync) 
        : device(pDevice->GetVkDevice()), physicalDevice(pDevice->GetVkPhysicalDevice()), queue(pQueue->GetVkQueue()), window(pWindow), imageCount(imageCount),
    vsync(vsync), currentFrame(0), currentImage(0) {
    auto instance = pInstance->GetVkInstance();
    VkSurfaceKHR tmpSurface;
    if(!SDL_Vulkan_CreateSurface(window, **instance, &tmpSurface)) {
        throw CVulkanSwapchainCreationException(EVulkanSwapchainCreationError::SURFACE_CREATION_FAILED);
    }
    surface = std::make_unique<vk::raii::SurfaceKHR>(*instance, tmpSurface);

    if(!physicalDevice.getSurfaceSupportKHR(0, **surface)) {
        throw CVulkanSwapchainCreationException(EVulkanSwapchainCreationError::SURFACE_PRESENTATION_NOT_SUPPORTED);
    }

    createSwapchain();
    createImageViews();

    for(uint8_t i = 0; i < imageCount; i++) {
        acquireFences.push_back(std::make_shared<vk::raii::Fence>(*device, vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled)));
    }

    for(uint8_t i = 0; i < imageCount; i++) {
        acquireSemaphores.push_back(std::make_shared<vk::raii::Semaphore>(*device, vk::SemaphoreCreateInfo()));
        submitSemaphores.push_back(std::make_shared<vk::raii::Semaphore>(*device, vk::SemaphoreCreateInfo()));
    }
}

CVulkanFrame CVulkanSwapchain::GetNextFrame() {
    vk::Result waitForFencesResult = device->waitForFences(**acquireFences[currentFrame], true, std::numeric_limits<uint64_t>::max());
    if(waitForFencesResult == vk::Result::eSuccess) {
        device->resetFences(**acquireFences.at(currentFrame));
        std::pair<vk::Result,uint32_t> acquireNextImageResultValue = swapchain->acquireNextImage(std::numeric_limits<uint64_t>::max(), **acquireSemaphores[currentFrame]);
        vk::Result acquireNextImageResult = acquireNextImageResultValue.first;
        if(acquireNextImageResult == vk::Result::eSuccess) {
            currentImage = acquireNextImageResultValue.second;
        } else if(acquireNextImageResult == vk::Result::eErrorSurfaceLostKHR) {
            printf("CVulkanSwapchain::AcquireNextImage: Lost Surface\n");
        } else if(acquireNextImageResult == vk::Result::eErrorOutOfDateKHR || acquireNextImageResult == vk::Result::eSuboptimalKHR) {
            printf("CVulkanSwapchain::AcquireNextImage: Swapchain needs recreation\n");
            Recreate();
        }
    } else if(waitForFencesResult == vk::Result::eTimeout) {
        printf("CVulkanSwapchain::AcquireNextImage: Waiting for fence timed out.\n");
    }

    CVulkanFrame frame;
    frame.currentImage = currentImage;
    frame.currentFrame = currentFrame;
    frame.extent = capabilities.currentExtent;
    frame.image = images[currentImage];
    frame.imageView = **imageViews[currentImage];
    frame.acquireFence = **acquireFences[currentFrame];
    frame.acquireSemaphore = **acquireSemaphores[currentFrame];
    frame.submitSemaphore = **submitSemaphores[currentFrame];
    return frame;
}

void CVulkanSwapchain::Present() {
    auto presentInfo = vk::PresentInfoKHR(**submitSemaphores[currentFrame], **swapchain, currentImage);
    vk::Result presentResult = queue->presentKHR(presentInfo);
    if(presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR) {
        printf("CVulkanSwapchain::Present: Swapchain needs recreation");
        Recreate();
    } else if(presentResult != vk::Result::eSuccess) {
        printf("CVulkanSwapchain::Present: Failed to present");
    }
    currentFrame = (currentFrame + 1) % imageCount;
}

void CVulkanSwapchain::Recreate() {
    device->waitIdle();
    createSwapchain();
    createImageViews();
}

vk::SurfaceCapabilitiesKHR CVulkanSwapchain::GetVkSurfaceCapabilities() {
    return capabilities;
}

vk::Format CVulkanSwapchain::GetVkSurfaceFormat() {
    return surfaceFormat.format;
}

void CVulkanSwapchain::createSwapchain() {
    capabilities = physicalDevice.getSurfaceCapabilitiesKHR(**surface);
    auto extent = GetSwapchainExtent(window, capabilities);
    presentMode = SelectPresentMode(physicalDevice.getSurfacePresentModesKHR(**surface), vsync ? vk::PresentModeKHR::eImmediate : vk::PresentModeKHR::eMailbox);
    surfaceFormat = SelectSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(**surface), vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear); // ImGui will use these settings, match them.

    vk::SwapchainCreateInfoKHR swapchainInfo;
    swapchainInfo.setSurface(**surface);
    swapchainInfo.setMinImageCount(imageCount);
    swapchainInfo.setImageFormat(surfaceFormat.format);
    swapchainInfo.setImageColorSpace(surfaceFormat.colorSpace);
    swapchainInfo.setImageExtent(extent);
    swapchainInfo.setImageArrayLayers(1);
    swapchainInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
    swapchainInfo.setImageSharingMode(vk::SharingMode::eExclusive);
    swapchainInfo.setPreTransform(capabilities.currentTransform);
    swapchainInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
    swapchainInfo.setPresentMode(presentMode);
    swapchainInfo.setClipped(true);

    swapchain = std::make_unique<vk::raii::SwapchainKHR>(*device, swapchainInfo);
}

void CVulkanSwapchain::createImageViews() {
    if(imageViews.size() > 0) {
        imageViews.clear();
    }

    images = swapchain->getImages();
    for(auto& image : images) {
        vk::ImageViewCreateInfo imageViewInfo;
        imageViewInfo.setImage(image);
        imageViewInfo.setViewType(vk::ImageViewType::e2D);
        imageViewInfo.setFormat(surfaceFormat.format);
        imageViewInfo.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
        imageViews.push_back(std::make_shared<vk::raii::ImageView>(*device, imageViewInfo));
    }
}

vk::SurfaceFormatKHR CVulkanSwapchain::SelectSurfaceFormat(std::vector<vk::SurfaceFormatKHR> surfaceFormats, vk::Format preferredFormat, vk::ColorSpaceKHR preferredColorSpace) {
    for(auto surfaceFormat : surfaceFormats) {
        if(surfaceFormat.format == preferredFormat && surfaceFormat.colorSpace == preferredColorSpace) {
            return surfaceFormat;
        }
    }
    return surfaceFormats.at(0);
}

vk::PresentModeKHR CVulkanSwapchain::SelectPresentMode(std::vector<vk::PresentModeKHR> presentModes, vk::PresentModeKHR preferred) {
    for(auto presentMode : presentModes) {
        if(presentMode == preferred) {
            return preferred;
        }
    }
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D CVulkanSwapchain::GetSwapchainExtent(SDL_Window* window, vk::SurfaceCapabilitiesKHR surfaceCapabilities) {
    auto currentExtent = surfaceCapabilities.currentExtent;
    // Use default extent we get from the surface, if it is not the max value(0xFFFFFFFF)
    if(currentExtent.width != std::numeric_limits<uint32_t>::max() &&
        currentExtent.height != std::numeric_limits<uint32_t>::max()) {
        return currentExtent;
    }

    // If we get max values, create the extent to match the window size manually, then adjust based on the minimum extent supported if it is too small.
    int width;
    int height;
    SDL_GetWindowSize(window, &width, &height);

    vk::Extent2D extent = { static_cast<uint8_t>(width), static_cast<uint8_t>(height) };
    auto minExtent = surfaceCapabilities.minImageExtent;
    auto maxExtent = surfaceCapabilities.maxImageExtent;
    extent.width = std::min(maxExtent.width, std::max(minExtent.width, extent.width));
    extent.height = std::min(maxExtent.height, std::max(minExtent.height, extent.width));
    return extent;
}
