module;
#include "platform/vulkan.hpp"
#include "platform/SDL.hpp"
#include <glm/glm.hpp>
export module renderer;
import <vector>;
import instance;
import device;
import queue;
import swapchain;
import buffer;
import cmd;
import pipeline;

export struct CVulkanRendererProperties {
    bool vsync;
    bool tripleBuffering;   
};

std::vector<CVulkanVertex> vertices = {
    CVulkanVertex(glm::vec2(0.0f, -0.5f), glm::vec3(1.0, .0f, 0.0f)),
    CVulkanVertex(glm::vec2(0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f)),
    CVulkanVertex(glm::vec2(-0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f)),
};

std::vector<uint16_t> indices = {
    0, 1, 2
};

export class CVulkanRenderer {
    CVulkanInstance instance;
    CVulkanDevice device;
    CVulkanSwapchain swapchain;
    CVulkanCommandPool graphicsCommandPool;
    CVulkanCommandPool transferCommandPool;
    std::vector<CVulkanCommandBuffer> graphicsCommandBuffers;
    CVulkanGraphicsPipeline pipeline;
    CVulkanBuffer vertexBuffer;
    CVulkanBuffer indexBuffer;

    // Let these get destroyed first, so any pending submissions complete.
    CVulkanQueue graphicsQueue;
    CVulkanQueue computeQueue;
    CVulkanQueue transferQueue;
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

        int imageCount = 2;
        swapchain = CVulkanSwapchain(vkInstance, vkPhysicalDevice, vkDevice, graphicsQueue.GetVkQueue(), window, imageCount, true);

        graphicsCommandPool = CVulkanCommandPool(vkDevice, graphicsQueue.GetFamilyIndex());
        auto vkGraphicsCommandPool = graphicsCommandPool.GetVkCommandPool();

        transferCommandPool = CVulkanCommandPool(vkDevice, transferQueue.GetFamilyIndex(), vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
        auto vkTransferCommandPool = transferCommandPool.GetVkCommandPool();
 
        for(int i = 0; i < imageCount; i++) {
            graphicsCommandBuffers.push_back(CVulkanCommandBuffer(vkDevice, vkGraphicsCommandPool));
        }

        CVulkanCommandBuffer transferCommandBuffer = CVulkanCommandBuffer(vkDevice, vkTransferCommandPool);

        auto surfaceFormat = swapchain.GetVkSurfaceFormat();
        auto surfaceCapabilities = swapchain.GetVkSurfaceCapabilities();
        pipeline = CVulkanGraphicsPipeline(vkDevice, "vertex.spv", "fragment.spv", surfaceFormat.format);

        // Vertex Buffer
        vk::DeviceSize vertexBufferSize = sizeof(CVulkanVertex) * vertices.size();
        CVulkanBuffer stagingVertexBuffer(vkDevice, device.GetVkPhysicalDeviceMemoryProperties(), vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, vk::BufferUsageFlagBits::eTransferSrc, vertices.data(), vertexBufferSize);
        vertexBuffer = CVulkanBuffer(vkDevice, device.GetVkPhysicalDeviceMemoryProperties(), vk::MemoryPropertyFlagBits::eDeviceLocal, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, nullptr, vertexBufferSize);
        
        transferCommandBuffer.CopyBuffer(stagingVertexBuffer.GetVkBuffer(), vertexBuffer.GetVkBuffer(), vk::BufferCopy(0, 0, vertexBufferSize));
        transferQueue.Submit(transferCommandBuffer.GetVkCommandBuffer());
        transferCommandBuffer.Reset();

        // Index Buffer
        vk::DeviceSize indexBufferSize = sizeof(indices[0]) * indices.size();
        CVulkanBuffer stagingIndexBuffer(vkDevice, device.GetVkPhysicalDeviceMemoryProperties(), vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, vk::BufferUsageFlagBits::eTransferSrc, indices.data(), indexBufferSize);
        indexBuffer = CVulkanBuffer(vkDevice, device.GetVkPhysicalDeviceMemoryProperties(), vk::MemoryPropertyFlagBits::eDeviceLocal, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, nullptr, indexBufferSize);

        transferCommandBuffer.CopyBuffer(stagingIndexBuffer.GetVkBuffer(), indexBuffer.GetVkBuffer(), vk::BufferCopy(0, 0, indexBufferSize));
        transferQueue.Submit(transferCommandBuffer.GetVkCommandBuffer());
        transferCommandBuffer.Reset();
    }

    void OnResize() {
        swapchain.Recreate();
    }

    void DrawFrame() {
        CVulkanFrame frame = swapchain.GetNextFrame();
        graphicsCommandPool.Reset();

        CVulkanRender render;
        render.colorAttachments = { vk::RenderingAttachmentInfo(frame.imageView, vk::ImageLayout::eColorAttachmentOptimal,
                                                            vk::ResolveModeFlagBits::eNone, nullptr, vk::ImageLayout::eUndefined,
                                                            vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
                                                           vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)) };
        /*
        render.setDepthAttachments = { vk::RenderingAttachmentInfo({}, vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                                                     vk::ResolveModeFlagBits::eNone, {}, vk::ImageLayout::eUndefined,
                                                                     vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
                                                                     vk::ClearDepthStencilValue(1.0f, 0)) };
        */
        render.pipeline = pipeline.GetVkPipeline();
        render.verticesCount = static_cast<uint32_t>(vertices.size());
        render.vertexBuffers = { vertexBuffer.GetVkBuffer() };
        render.vertexBufferOffsets = { 0 };
        render.indicesCount = static_cast<uint32_t>(indices.size());
        render.indexBuffer = indexBuffer.GetVkBuffer();
        render.indexBufferOffset = 0;

        graphicsCommandBuffers[frame.currentFrame].Render(frame, render);
        graphicsQueue.Submit(graphicsCommandBuffers[frame.currentFrame].GetVkCommandBuffer(), frame.submitSemaphore, frame.acquireSemaphore, vk::PipelineStageFlagBits::eColorAttachmentOutput, frame.acquireFence);
        swapchain.Present();
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