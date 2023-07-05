module;
#include "platform/vulkan.hpp"
#include "platform/SDL.hpp"
export module renderer;
import <vector>;
import instance;
import device;
import queue;
import swapchain;
import renderpass;
import buffer;
import framebuffer;
import cmd;
import shader;

export class CVulkanRenderer {
    CVulkanInstance instance;
    CVulkanDevice device;
    CVulkanQueue graphicsQueue;
    CVulkanQueue computeQueue;
    CVulkanQueue transferQueue;
    CVulkanSwapchain swapchain;
    CVulkanRenderPass renderPass;
    std::vector<CVulkanFramebuffer> framebuffers;
    CVulkanBufferAllocator bufferAllocator;
    CVulkanCommandPool commandPool;
    CVulkanCommandBuffer commandBuffer;
public:
    CVulkanRenderer() = default;
    CVulkanRenderer(SDL_Window* window) {
        instance = CVulkanInstance(window);
        
        auto vkInstance = instance.GetVkInstance();
        auto vkPhysicalDevices = instance.GetVkPhysicalDevices();
        auto vkPhysicalDevice = SelectPrimaryPhysicalDevice(vkPhysicalDevices);
        device = CVulkanDevice(vkPhysicalDevice);
        auto vkDevice = device.GetVkDevice();
        
        int imageCount = 2;
        graphicsQueue = CVulkanQueue(vkDevice, device.GetGraphicsQueueIndex(), imageCount);
        computeQueue = CVulkanQueue(vkDevice, device.GetComputeQueueIndex());
        transferQueue = CVulkanQueue(vkDevice, device.GetTransferQueueIndex());
        
        swapchain = CVulkanSwapchain(vkInstance, vkPhysicalDevice, vkDevice, graphicsQueue.GetVkQueue(), window, 2);
        auto vkImages = swapchain.GetVkImages();
        auto surfaceFormat = swapchain.GetVkSurfaceFormat().format;
        auto surfaceExtent = swapchain.GetVkSurfaceCapabilities().currentExtent;

        renderPass = CVulkanRenderPass(vkDevice, swapchain.GetVkSurfaceFormat().format);
        auto vkRenderPass = renderPass.GetVkRenderPass();

        for(auto& vkImage : vkImages) {
            framebuffers.push_back(CVulkanFramebuffer(vkDevice, vkRenderPass, vkImage, surfaceFormat, surfaceExtent));
        }

        bufferAllocator = CVulkanBufferAllocator(vkInstance, vkPhysicalDevice, vkDevice);

        commandPool = CVulkanCommandPool(vkDevice, graphicsQueue.GetFamilyIndex());
        auto vkCommandPool = commandPool.GetVkCommandPool();

        commandBuffer = CVulkanCommandBuffer(vkDevice, vkCommandPool);
    }

    CVulkanRenderer(const CVulkanRenderer&) = default;
    CVulkanRenderer(CVulkanRenderer&&) = default;
    CVulkanRenderer& operator=(const CVulkanRenderer&) = default;
    CVulkanRenderer& operator=(CVulkanRenderer&& device) = default;

    void OnResize() {
        swapchain.Recreate();
    }

    void DrawFrame() {
        auto vkDevice = device.GetVkDevice();

        //uint32_t image = swapchain.AcquireNextImage();
        //swapchain.Present(graphicsQueue.GetVkSubmitSemaphore());
    }

    // Hook up events to the renderer.
    static int SDL_EventFilterCallback(void* userdata, SDL_Event* event) {
        CVulkanRenderer* renderer = static_cast<CVulkanRenderer*>(userdata);
        if(renderer != nullptr) {
            if(event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED) {
                printf("CVulkanRenderer::OnWindowResizeSDL_EventCallback: Performing Resize\n");
                renderer->OnResize();
            }
        }
        return 0;
    }

private:
    vk::PhysicalDevice SelectPrimaryPhysicalDevice(std::vector<vk::PhysicalDevice> physicalDevices) {
        for(auto& physicalDevice : physicalDevices) {
            vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
            if(properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
                return physicalDevice;
            }
        }
        return physicalDevices.front();
    }
};