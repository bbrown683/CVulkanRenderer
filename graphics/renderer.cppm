module;
#include "platform/vulkan.hpp"
#include "platform/sdl.hpp"
export module renderer;
import <vector>;
import <map>;
import instance;
import device;
import queue;
import swapchain;
import renderpass;
import buffer;

export class CVulkanRenderer {
    CVulkanInstance instance;
    CVulkanDevice device;
    CVulkanQueue graphicsQueue;
    CVulkanQueue computeQueue;
    CVulkanQueue transferQueue;
    CVulkanSwapchain swapchain;
    CVulkanRenderPass renderPass;
    CVulkanBufferAllocator bufferAllocator;
public:
    CVulkanRenderer(SDL_Window* window) {
        instance = CVulkanInstance(window);
        auto vkInstance = instance.GetVkInstance();
        auto vkPhysicalDevices = instance.GetVkPhysicalDevices();
        auto vkPhysicalDevice = SelectPrimaryPhysicalDevice(vkPhysicalDevices);
        device = CVulkanDevice(vkPhysicalDevice);
        auto vkDevice = device.GetVkDevice();
        graphicsQueue = CVulkanQueue(vkDevice, device.GetGraphicsQueueIndex());
        computeQueue = CVulkanQueue(vkDevice, device.GetComputeQueueIndex());
        transferQueue = CVulkanQueue(vkDevice, device.GetTransferQueueIndex());
        swapchain = CVulkanSwapchain(vkInstance, vkPhysicalDevice, vkDevice, graphicsQueue.GetVkQueue(), window, 2);
        renderPass = CVulkanRenderPass(vkDevice, swapchain.GetVkSurfaceFormat().format);
        bufferAllocator = CVulkanBufferAllocator(vkInstance, vkPhysicalDevice, vkDevice);
    }

    void OnResize() {
        swapchain.Recreate();
    }

    // Recreate the Swapchain when we receive the corresponding SDL event.
    static int OnResizeSDL_EventCallback(void* userdata, SDL_Event* event) {
        CVulkanRenderer* renderer = static_cast<CVulkanRenderer*>(userdata);
        if(renderer != nullptr) {
            if(event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED) {
                printf("CVulkanRenderer::OnResizeSDL_EventCallback: Performing Resize\n");
                renderer->OnResize();
            }
        }
        return 0;
    }

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