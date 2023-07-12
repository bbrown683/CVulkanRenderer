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
import ui;

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
    std::shared_ptr<CVulkanInstance> instance;
    std::shared_ptr<CVulkanDevice> device;
    std::unique_ptr<CVulkanSwapchain> swapchain;
    std::unique_ptr<CVulkanCommandPool> graphicsCommandPool;
    std::unique_ptr<CVulkanCommandPool> transferCommandPool;
    std::vector<std::shared_ptr<CVulkanCommandBuffer>> graphicsCommandBuffers;
    std::unique_ptr<CVulkanGraphicsPipeline> pipeline;
    std::unique_ptr<CVulkanBuffer> vertexBuffer;
    std::unique_ptr<CVulkanBuffer> indexBuffer;
    std::unique_ptr<CVulkanUi> ui;

    // Let these get destroyed first, so any pending submissions complete.
    std::shared_ptr<CVulkanQueue> graphicsQueue;
    std::shared_ptr<CVulkanQueue> computeQueue;
    std::shared_ptr<CVulkanQueue> transferQueue;
public:
    CVulkanRenderer(SDL_Window* window) {
        instance = std::make_shared<CVulkanInstance>(window);
        auto vkInstance = instance->GetVkInstance();
        auto vkPhysicalDevices = instance->GetVkPhysicalDevices();
        auto vkPhysicalDevice = SelectPrimaryPhysicalDevice(vkPhysicalDevices);
        device = std::make_shared<CVulkanDevice>(vkPhysicalDevice);
        auto vkDevice = device->GetVkDevice();

        graphicsQueue = std::make_shared<CVulkanQueue>(device, device->GetGraphicsQueueIndex());
        computeQueue = std::make_shared<CVulkanQueue>(device, device->GetComputeQueueIndex());
        transferQueue = std::make_shared<CVulkanQueue>(device, device->GetTransferQueueIndex());

        swapchain = std::make_unique<CVulkanSwapchain>(instance->GetVkInstance(), vkPhysicalDevice, vkDevice, graphicsQueue->GetVkQueue(), window, 2, true);

        graphicsCommandPool = std::make_unique<CVulkanCommandPool>(vkDevice, graphicsQueue->GetFamilyIndex());
        auto vkGraphicsCommandPool = graphicsCommandPool->GetVkCommandPool();

        transferCommandPool = std::make_unique<CVulkanCommandPool>(vkDevice, transferQueue->GetFamilyIndex(), vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
        auto vkTransferCommandPool = transferCommandPool->GetVkCommandPool();
 
        for(int i = 0; i < swapchain->GetImageCount(); i++) {
            graphicsCommandBuffers.push_back(std::make_shared<CVulkanCommandBuffer>(vkDevice, vkGraphicsCommandPool));
        }

        CVulkanCommandBuffer transferCommandBuffer(vkDevice, vkTransferCommandPool);

        auto surfaceFormat = swapchain->GetVkSurfaceFormat();
        auto surfaceCapabilities = swapchain->GetVkSurfaceCapabilities();
        pipeline = std::make_unique<CVulkanGraphicsPipeline>(vkDevice, "vertex.spv", "fragment.spv", surfaceFormat.format);

        auto memoryProperties = device->GetVkPhysicalDeviceMemoryProperties();

        // Vertex Buffer
        vk::DeviceSize vertexBufferSize = sizeof(CVulkanVertex) * vertices.size();
        CVulkanBuffer stagingVertexBuffer(vkDevice, memoryProperties, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, vk::BufferUsageFlagBits::eTransferSrc, vertices.data(), vertexBufferSize);
        vertexBuffer = std::make_unique<CVulkanBuffer>(vkDevice, memoryProperties, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, nullptr, vertexBufferSize);
        
        transferCommandBuffer.CopyBuffer(stagingVertexBuffer.GetVkBuffer(), vertexBuffer->GetVkBuffer(), vk::BufferCopy(0, 0, vertexBufferSize));
        transferQueue->Submit(transferCommandBuffer.GetVkCommandBuffer());
        transferCommandBuffer.Reset();

        // Index Buffer
        vk::DeviceSize indexBufferSize = sizeof(indices[0]) * indices.size();
        CVulkanBuffer stagingIndexBuffer(vkDevice, memoryProperties, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, vk::BufferUsageFlagBits::eTransferSrc, indices.data(), indexBufferSize);
        indexBuffer = std::make_unique<CVulkanBuffer>(vkDevice, memoryProperties, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, nullptr, indexBufferSize);

        transferCommandBuffer.CopyBuffer(stagingIndexBuffer.GetVkBuffer(), indexBuffer->GetVkBuffer(), vk::BufferCopy(0, 0, indexBufferSize));
        transferQueue->Submit(transferCommandBuffer.GetVkCommandBuffer());
        transferCommandBuffer.Reset();

        ui = std::make_unique<CVulkanUi>(window, instance.get(), device.get(), graphicsQueue.get(), graphicsCommandBuffers[0].get(), 2, surfaceFormat.format);
    }

    void OnResize() {
        swapchain->Recreate();
    }

    void DrawFrame() {
        CVulkanFrame frame = swapchain->GetNextFrame();
        graphicsCommandPool->Reset();

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
        render.pipeline = pipeline->GetVkPipeline();
        render.verticesCount = static_cast<uint32_t>(vertices.size());
        render.vertexBuffers = { vertexBuffer->GetVkBuffer() };
        render.vertexBufferOffsets = { 0 };
        render.indicesCount = static_cast<uint32_t>(indices.size());
        render.indexBuffer = indexBuffer->GetVkBuffer();
        render.indexBufferOffset = 0;

        graphicsCommandBuffers[frame.currentFrame]->Render(frame, render);
        graphicsQueue->Submit(graphicsCommandBuffers[frame.currentFrame]->GetVkCommandBuffer(), frame.submitSemaphore, frame.acquireSemaphore, vk::PipelineStageFlagBits::eColorAttachmentOutput, frame.acquireFence);
        swapchain->Present();
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