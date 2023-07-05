module;
#include "platform/vulkan.hpp"
#include "platform/SDL.hpp"
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
    SDL_Window* window;
    vk::UniqueSurfaceKHR surface;
    vk::SurfaceFormatKHR surfaceFormat;
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    vk::PresentModeKHR presentMode;
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
            : physicalDevice(physicalDevice), device(device), queue(queue), window(window), imageCount(imageCount), currentFrame(0), currentImage(0) {
        VkSurfaceKHR tmpSurface;
        if(!SDL_Vulkan_CreateSurface(window, instance, &tmpSurface)) {
            throw CVulkanSwapchainCreationException(EVulkanSwapchainCreationError::SURFACE_CREATION_FAILED);
        }
        surface = vk::UniqueSurfaceKHR(tmpSurface, instance);

        if(!physicalDevice.getSurfaceSupportKHR(0, *surface)) {
            throw CVulkanSwapchainCreationException(EVulkanSwapchainCreationError::SURFACE_PRESENTATION_NOT_SUPPORTED);
        }

        capabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
        auto extent = GetSwapchainExtent(window, capabilities);
        presentMode = SelectPresentMode(physicalDevice.getSurfacePresentModesKHR(*surface), vk::PresentModeKHR::eMailbox);
        surfaceFormat = SelectSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(*surface), vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear);
        auto swapchainInfo = vk::SwapchainCreateInfoKHR(vk::SwapchainCreateFlagsKHR(), *surface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace,
                                                        extent, 1, vk::ImageUsageFlagBits::eColorAttachment, {}, nullptr,
                                                        vk::SurfaceTransformFlagBitsKHR::eIdentity, vk::CompositeAlphaFlagBitsKHR::eOpaque, presentMode, VK_TRUE);

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
                printf("CVulkanSwapchain::AcquireNextImage: Lost Surface\n");
            } else if(acquireNextImageResult == vk::Result::eErrorOutOfDateKHR) {
                printf("CVulkanSwapchain::AcquireNextImage: Images are out of date\n");
                Recreate();
            }
        } else if(waitForFencesResult == vk::Result::eTimeout) {
            printf("CVulkanSwapchain::AcquireNextImage: Waiting for fence timed out.\n");
        }
        return currentImage;
    }

    void Recreate() {
        device.waitIdle();
        capabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
        auto extent = GetSwapchainExtent(window, capabilities);
        presentMode = SelectPresentMode(physicalDevice.getSurfacePresentModesKHR(*surface), vk::PresentModeKHR::eMailbox);
        surfaceFormat = SelectSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(*surface), vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear);
        auto swapchainInfo = vk::SwapchainCreateInfoKHR(vk::SwapchainCreateFlagsKHR(), *surface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace,
                                                        extent, 1, vk::ImageUsageFlagBits::eColorAttachment, {}, nullptr,
                                                        vk::SurfaceTransformFlagBitsKHR::eIdentity, vk::CompositeAlphaFlagBitsKHR::eOpaque, presentMode, VK_TRUE, *swapchain);
        swapchain = device.createSwapchainKHRUnique(swapchainInfo);
    }

    // Presents the image to the screen, using the specified present queue. The present queue can be any queue
    // graphics, transfer, compute which supports present operations.
    void Present(vk::Semaphore submitSemaphore) {
        auto presentInfo = vk::PresentInfoKHR(submitSemaphore, *swapchain, currentImage);
        vk::Result presentResult = queue.presentKHR(presentInfo);
        if(presentResult == vk::Result::eErrorOutOfDateKHR) {
            printf("CVulkanSwapchain::Present: Swapchain is out of date.");
            Recreate();
        } else if(presentResult != vk::Result::eSuccess) {
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

    vk::PresentModeKHR GetVkPresentMode() {
        return presentMode;
    }

    vk::Semaphore GetCurrentAcquireSemaphore() {
        return *acquireSemaphores.at(currentFrame);
    }
    
    vk::Fence GetCurrentAcquireFence() {
        return *acquireFences.at(currentFrame);
    }
private:
    vk::SurfaceFormatKHR SelectSurfaceFormat(std::vector<vk::SurfaceFormatKHR> surfaceFormats, vk::Format preferredFormat, vk::ColorSpaceKHR preferredColorSpace) {
        for(auto surfaceFormat : surfaceFormats) {
            if(surfaceFormat.format == preferredFormat && surfaceFormat.colorSpace == preferredColorSpace) {
                return surfaceFormat;
            }
        }
        return surfaceFormats.at(0);
    }

    vk::PresentModeKHR SelectPresentMode(std::vector<vk::PresentModeKHR> presentModes, vk::PresentModeKHR preferred) {
        for(auto presentMode : presentModes) {
            if(presentMode == preferred) {
                return preferred;
            }
        }
        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D GetSwapchainExtent(SDL_Window* window, vk::SurfaceCapabilitiesKHR surfaceCapabilities) {
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
};