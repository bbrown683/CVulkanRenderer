module;
#include "platform/vulkan.hpp"
#include "platform/sdl.hpp"
module swapchain;

// On Failure can throw a SwapchainCreationException.
CVulkanSwapchain::CVulkanSwapchain(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device, vk::Queue presentQueue, SDL_Window* window, uint32_t imageCount)
    : instance(instance), device(device), presentQueue(presentQueue) {
    VkSurfaceKHR tmpSurface;
    if(!SDL_Vulkan_CreateSurface(window, instance, &tmpSurface)) {
        throw CVulkanSwapchainCreationException(EVulkanSwapchainCreationError::SURFACE_CREATION_FAILED);
    }
    surface = vk::UniqueSurfaceKHR(tmpSurface, instance);

    if(!physicalDevice.getSurfaceSupportKHR(0, *surface)) {
        throw CVulkanSwapchainCreationException(EVulkanSwapchainCreationError::SURFACE_PRESENTATION_NOT_SUPPORTED);
    }

    capabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
    presentModes = physicalDevice.getSurfacePresentModesKHR(*surface);
    surfaceFormat = SelectSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(*surface), vk::Format::eB8G8R8A8Srgb);
    auto swapchainInfo = vk::SwapchainCreateInfoKHR(vk::SwapchainCreateFlagsKHR(), *surface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace,
                                                    capabilities.currentExtent, 1, vk::ImageUsageFlagBits::eColorAttachment, {}, nullptr,
                                                    vk::SurfaceTransformFlagBitsKHR::eIdentity, vk::CompositeAlphaFlagBitsKHR::eOpaque, {}, VK_TRUE);

    swapchain = device.createSwapchainKHRUnique(swapchainInfo);

    for(uint8_t i = 0; i < imageCount; i++) {
        acquireSemaphores.push_back(device.createSemaphoreUnique({}));
    }

    auto fenceInfo = vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);
    for(uint8_t i = 0; i < imageCount; i++) {
        acquireFences.push_back(device.createFenceUnique(fenceInfo));
    }
    images = device.getSwapchainImagesKHR(*swapchain);
}

uint32_t CVulkanSwapchain::AcquireNextImage() {
    currentFrame = (currentFrame + 1) % imageCount;
    return currentImage;
}

bool CVulkanSwapchain::Recreate() {
    return false;
}

// Presents the image to the screen, using the specified present queue. The present queue can be any queue
// graphics, transfer, compute which supports present operations.
void CVulkanSwapchain::Present() {
    auto presentInfo = vk::PresentInfoKHR();
}

std::vector<vk::Image> CVulkanSwapchain::GetVkImages() {
    return images;
}

uint32_t CVulkanSwapchain::GetCurrentImage() {
    return currentImage;
}

vk::SurfaceCapabilitiesKHR CVulkanSwapchain::GetVkSurfaceCapabilities() {
    return capabilities;
}

std::vector<vk::SurfaceFormatKHR> CVulkanSwapchain::GetVkSurfaceFormats() {
    return formats;
}

vk::SurfaceFormatKHR CVulkanSwapchain::GetVkSurfaceFormat() {
    return surfaceFormat;
}

std::vector<vk::PresentModeKHR> CVulkanSwapchain::GetVkPresentModes() {
    return presentModes;
}

vk::Semaphore CVulkanSwapchain::GetCurrentAcquireSemaphore() {
    return *acquireSemaphores.at(currentFrame);
}

vk::Fence CVulkanSwapchain::GetCurrentAcquireFence() {
    return *acquireFences.at(currentFrame);
}

vk::SurfaceFormatKHR CVulkanSwapchain::SelectSurfaceFormat(std::vector<vk::SurfaceFormatKHR> surfaceFormats, vk::Format preferred) {
    for(auto surfaceFormat : surfaceFormats) {
        if(surfaceFormat.format == preferred) {
            return surfaceFormat;
        }
    }
    return surfaceFormats.at(0);
}


