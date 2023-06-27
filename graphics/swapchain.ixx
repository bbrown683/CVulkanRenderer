module;
#include "platform/vulkan.hpp"
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

export struct SDL_Window;

export class CVulkanSwapchain {
    vk::Instance instance;
    vk::Device device;
    vk::Queue presentQueue;
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
    // On Failure can throw a SwapchainCreationException.
    CVulkanSwapchain(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device, vk::Queue presentQueue, SDL_Window* window, uint32_t imageCount);
    uint32_t AcquireNextImage();
    bool Recreate();
    // Presents the image to the screen, using the specified present queue. The present queue can be any queue
    // graphics, transfer, compute which supports present operations.
    void Present();
    std::vector<vk::Image> GetVkImages();
    uint32_t GetCurrentImage();
    vk::SurfaceCapabilitiesKHR GetVkSurfaceCapabilities();
    std::vector<vk::SurfaceFormatKHR> GetVkSurfaceFormats();
    vk::SurfaceFormatKHR GetVkSurfaceFormat();
    std::vector<vk::PresentModeKHR> GetVkPresentModes();
    vk::Semaphore GetCurrentAcquireSemaphore();
    vk::Fence GetCurrentAcquireFence();
private:
    vk::SurfaceFormatKHR SelectSurfaceFormat(std::vector<vk::SurfaceFormatKHR> surfaceFormats, vk::Format preferred);
};