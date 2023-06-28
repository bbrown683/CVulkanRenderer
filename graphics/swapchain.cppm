module;
#include "platform/vulkan.hpp"
#include "platform/sdl.hpp"
export module swapchain;
import <exception>;
import <vector>;

export enum EVulkanSwapchainCreationError {
    SURFACE_CREATION_FAILED,
    SURFACE_PRESENTATION_NOT_SUPPORTED,
};

export class CVulkanSwapchainCreationException : public std::exception {
    EVulkanSwapchainCreationError error;
public:
    CVulkanSwapchainCreationException(EVulkanSwapchainCreationError error) : error(error) {}

    char* what() {
        return (char*)"Failed to create swapchain due to error: " + error;
    }

    EVulkanSwapchainCreationError GetError() {
        return error;
    }
};

export class CVulkanSwapchain {
    vk::PhysicalDevice physicalDevice;
    vk::Device device;
    vk::Queue queue;
    vk::UniqueSurfaceKHR surface;
    vk::SurfaceFormatKHR surfaceFormat;
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
    vk::UniqueSwapchainKHR swapchain;
    std::vector<vk::UniqueSemaphore> acquireSemaphores;
    std::vector<vk::UniqueFence> acquireFences;
    std::vector<vk::Image> images;
    uint32_t imageCount;
    uint32_t currentFrame;
    uint32_t currentImage;
public:
    CVulkanSwapchain() = default;
    // On Failure can throw a SwapchainCreationException.
    CVulkanSwapchain(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device, vk::Queue queue, SDL_Window* window, uint32_t imageCount) 
            : physicalDevice(physicalDevice), device(device), queue(queue) {
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

    uint32_t AcquireNextImage() {
        currentFrame = (currentFrame + 1) % imageCount;
        vk::Result waitForFencesResult = device.waitForFences(*acquireFences.at(currentFrame), VK_TRUE, std::numeric_limits<uint64_t>::max());
        if(waitForFencesResult == vk::Result::eSuccess) {
            device.resetFences(*acquireFences.at(currentFrame));
            vk::ResultValue<uint32_t> acquireNextImageResultValue = device.acquireNextImageKHR(*swapchain, std::numeric_limits<uint64_t>::max(), *acquireSemaphores.at(currentFrame));
            vk::Result acquireNextImageResult = acquireNextImageResultValue.result;
            if(acquireNextImageResult == vk::Result::eSuccess) {
                currentImage = acquireNextImageResultValue.value;
            } else if(acquireNextImageResult == vk::Result::eErrorSurfaceLostKHR) {
                printf("CVulkanSwapchain::AcquireNextImage: Lost Surface");
            } else if(acquireNextImageResult == vk::Result::eErrorOutOfDateKHR) {
                printf("CVulkanSwapchain::AcquireNextImage: Images are out of date");
            }
        }
        return currentImage;
    }

    void Recreate() {
        capabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
        presentModes = physicalDevice.getSurfacePresentModesKHR(*surface);
        surfaceFormat = SelectSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(*surface), vk::Format::eB8G8R8A8Srgb);
        auto swapchainInfo = vk::SwapchainCreateInfoKHR(vk::SwapchainCreateFlagsKHR(), *surface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace,
                                                        capabilities.currentExtent, 1, vk::ImageUsageFlagBits::eColorAttachment, {}, nullptr,
                                                        vk::SurfaceTransformFlagBitsKHR::eIdentity, vk::CompositeAlphaFlagBitsKHR::eOpaque, {}, VK_TRUE, *swapchain);
        swapchain = device.createSwapchainKHRUnique(swapchainInfo);
    }

    // Presents the image to the screen, using the specified present queue. The present queue can be any queue
    // graphics, transfer, compute which supports present operations.
    void Present(vk::Semaphore submitSemaphore) {
        auto presentInfo = vk::PresentInfoKHR(submitSemaphore, *swapchain, currentImage);
        vk::Result presentResult = queue.presentKHR(presentInfo);
        if(presentResult != vk::Result::eSuccess) {
            printf("CVulkanSwapchain::Present: Failed to present.");
        }
    }

    vk::SwapchainKHR GetVkSwapchain() {
        return *swapchain;
    }

    vk::SurfaceKHR GetVkSurface() {
        return *surface;
    }

    std::vector<vk::Image> GetVkImages() {
        return images;
    }
    
    uint32_t GetCurrentImage() {
        return currentImage;
    }

    vk::SurfaceCapabilitiesKHR GetVkSurfaceCapabilities() {
        return capabilities;
    }

    std::vector<vk::SurfaceFormatKHR> GetVkSurfaceFormats() {
        return formats;
    }

    vk::SurfaceFormatKHR GetVkSurfaceFormat() {
        return surfaceFormat;
    }

    std::vector<vk::PresentModeKHR> GetVkPresentModes() {
        return presentModes;
    }

    vk::Semaphore GetCurrentAcquireSemaphore() {
        return *acquireSemaphores.at(currentFrame);
    }
    
    vk::Fence GetCurrentAcquireFence() {
        return *acquireFences.at(currentFrame);
    }
private:
    vk::SurfaceFormatKHR SelectSurfaceFormat(std::vector<vk::SurfaceFormatKHR> surfaceFormats, vk::Format preferred) {
        for(auto surfaceFormat : surfaceFormats) {
            if(surfaceFormat.format == preferred) {
                return surfaceFormat;
            }
        }
        return surfaceFormats.at(0);
    }
};