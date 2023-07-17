#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <exception>

struct SDL_Window;
class CVulkanInstance;
class CVulkanDevice;
class CVulkanQueue;
struct CVulkanFrame;

enum EVulkanSwapchainCreationError {
    SURFACE_CREATION_FAILED,
    SURFACE_PRESENTATION_NOT_SUPPORTED,
};

class CVulkanSwapchainCreationException : public std::exception {
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

class CVulkanSwapchain {
    std::shared_ptr<vk::raii::Device> device;
    vk::PhysicalDevice physicalDevice;
    std::shared_ptr<vk::raii::Queue> queue;
    SDL_Window* window;
    std::unique_ptr<vk::raii::SurfaceKHR> surface;
    vk::SurfaceFormatKHR surfaceFormat;
    vk::SurfaceCapabilitiesKHR capabilities;
    vk::PresentModeKHR presentMode;
    std::unique_ptr<vk::raii::SwapchainKHR> swapchain;
    std::vector<std::shared_ptr<vk::raii::Fence>> acquireFences;
    std::vector<std::shared_ptr<vk::raii::Semaphore>> acquireSemaphores;
    std::vector<std::shared_ptr<vk::raii::Semaphore>> submitSemaphores;
    std::vector<vk::Image> images;
    std::vector<std::shared_ptr<vk::raii::ImageView>> imageViews;
    uint32_t imageCount;
    uint32_t currentFrame;
    uint32_t currentImage;
    bool vsync;
public:
    // On Failure can throw a SwapchainCreationException.
    CVulkanSwapchain(CVulkanInstance* pInstance, CVulkanDevice* pDevice, CVulkanQueue* pQueue, SDL_Window* pWindow, uint32_t imageCount, bool vsync = true);
    CVulkanFrame GetNextFrame();
    // Presents the image to the screen, using the specified present presentQueue. The present presentQueue can be any presentQueue
    // graphics, transfer, compute which supports present operations.
    void Present();
    void Recreate();
    vk::SurfaceCapabilitiesKHR GetVkSurfaceCapabilities();
    vk::Format GetVkSurfaceFormat();
private:
    void createSwapchain();
    void createImageViews();
    vk::SurfaceFormatKHR SelectSurfaceFormat(std::vector<vk::SurfaceFormatKHR> surfaceFormats, vk::Format preferredFormat, vk::ColorSpaceKHR preferredColorSpace);
    vk::PresentModeKHR SelectPresentMode(std::vector<vk::PresentModeKHR> presentModes, vk::PresentModeKHR preferred);
    vk::Extent2D GetSwapchainExtent(SDL_Window* window, vk::SurfaceCapabilitiesKHR surfaceCapabilities);
};