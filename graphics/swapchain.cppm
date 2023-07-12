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

// Data used every frame.
export struct CVulkanFrame {
    uint32_t currentImage;
    uint32_t currentFrame;
    vk::Extent2D extent;
    vk::Image image;
    vk::ImageView imageView;
    vk::Fence acquireFence;
    vk::Semaphore acquireSemaphore;
    vk::Semaphore submitSemaphore;
};

export class CVulkanSwapchain {
    vk::PhysicalDevice physicalDevice;
    vk::Device device;
    vk::Queue presentQueue;
    SDL_Window* window;
    vk::UniqueSurfaceKHR surface;
    vk::SurfaceFormatKHR surfaceFormat;
    vk::SurfaceCapabilitiesKHR capabilities;
    vk::PresentModeKHR presentMode;
    vk::UniqueSwapchainKHR swapchain;
    std::vector<vk::UniqueFence> acquireFences;
    std::vector<vk::UniqueSemaphore> acquireSemaphores;
    std::vector<vk::UniqueSemaphore> submitSemaphores;
    std::vector<vk::Image> images;
    std::vector<vk::UniqueImageView> imageViews;
    uint32_t imageCount;
    uint32_t currentFrame;
    uint32_t currentImage;
    bool vsync;
public:
    CVulkanSwapchain() = default;
    // On Failure can throw a SwapchainCreationException.
    CVulkanSwapchain(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device, vk::Queue presentQueue, SDL_Window* window, uint32_t imageCount, bool vsync = true)
            : physicalDevice(physicalDevice), device(device), presentQueue(presentQueue), window(window), imageCount(imageCount), vsync(vsync), currentFrame(0), currentImage(0) {
        VkSurfaceKHR tmpSurface;
        if(!SDL_Vulkan_CreateSurface(window, instance, &tmpSurface)) {
            throw CVulkanSwapchainCreationException(EVulkanSwapchainCreationError::SURFACE_CREATION_FAILED);
        }
        surface = vk::UniqueSurfaceKHR(tmpSurface, instance);

        if(!physicalDevice.getSurfaceSupportKHR(0, *surface)) {
            throw CVulkanSwapchainCreationException(EVulkanSwapchainCreationError::SURFACE_PRESENTATION_NOT_SUPPORTED);
        }

        createSwapchain();
        createImageViews();

        for(uint8_t i = 0; i < imageCount; i++) {
            acquireFences.push_back(device.createFenceUnique(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled)));
        }

        for(uint8_t i = 0; i < imageCount; i++) {
            acquireSemaphores.push_back(device.createSemaphoreUnique({}));
            submitSemaphores.push_back(device.createSemaphoreUnique({}));
        }
    }

    CVulkanFrame GetNextFrame() {
        vk::Result waitForFencesResult = device.waitForFences(*acquireFences[currentFrame], true, std::numeric_limits<uint64_t>::max());
        if(waitForFencesResult == vk::Result::eSuccess) {
            device.resetFences(*acquireFences.at(currentFrame));
            vk::ResultValue<uint32_t> acquireNextImageResultValue = device.acquireNextImageKHR(*swapchain, std::numeric_limits<uint64_t>::max(), *acquireSemaphores[currentFrame]);
            vk::Result acquireNextImageResult = acquireNextImageResultValue.result;
            if(acquireNextImageResult == vk::Result::eSuccess) {
                currentImage = acquireNextImageResultValue.value;
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
        frame.imageView = *imageViews[currentImage];
        frame.acquireFence = *acquireFences[currentFrame];
        frame.acquireSemaphore = *acquireSemaphores[currentFrame];
        frame.submitSemaphore = *submitSemaphores[currentFrame];
        return frame;
    }

    // Presents the image to the screen, using the specified present presentQueue. The present presentQueue can be any presentQueue
    // graphics, transfer, compute which supports present operations.
    void Present() {
        auto presentInfo = vk::PresentInfoKHR(*submitSemaphores[currentFrame], *swapchain, currentImage);
        vk::Result presentResult = presentQueue.presentKHR(presentInfo);
        if(presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR) {
            printf("CVulkanSwapchain::Present: Swapchain needs recreation");
            Recreate();
        } else if(presentResult != vk::Result::eSuccess) {
            printf("CVulkanSwapchain::Present: Failed to present");
        }
        currentFrame = (currentFrame + 1) % imageCount;
    }

    void Recreate() {
        device.waitIdle();
        createSwapchain();
        createImageViews();
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

    vk::SurfaceFormatKHR GetVkSurfaceFormat() {
        return surfaceFormat;
    }

    vk::PresentModeKHR GetVkPresentMode() {
        return presentMode;
    }

    vk::Semaphore GetCurrentAcquireSemaphore() {
        return *acquireSemaphores[currentFrame];
    }
    
    vk::Semaphore GetCurrentSubmitSemaphore() {
        return *submitSemaphores[currentFrame];
    }

    vk::Fence GetCurrentAcquireFence() {
        return *acquireFences[currentFrame];
    }

    vk::ImageView GetCurrentImageView() {
        return *imageViews[currentImage];
    }
private:
    void createSwapchain() {
        capabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
        auto extent = GetSwapchainExtent(window, capabilities);
        presentMode = SelectPresentMode(physicalDevice.getSurfacePresentModesKHR(*surface), vsync ? vk::PresentModeKHR::eImmediate : vk::PresentModeKHR::eMailbox);
        surfaceFormat = SelectSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(*surface), vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear);

        vk::SwapchainCreateInfoKHR swapchainInfo;
        swapchainInfo.setSurface(*surface);
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

        swapchain = device.createSwapchainKHRUnique(swapchainInfo);
    }

    void createImageViews() {
        if(imageViews.size() > 0) {
            imageViews.clear();
        }

        images = device.getSwapchainImagesKHR(*swapchain);
        for(auto& image : images) {
            vk::ImageViewCreateInfo imageViewInfo;
            imageViewInfo.setImage(image);
            imageViewInfo.setViewType(vk::ImageViewType::e2D);
            imageViewInfo.setFormat(surfaceFormat.format);
            imageViewInfo.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
            imageViews.push_back(device.createImageViewUnique(imageViewInfo));
        }
    }

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