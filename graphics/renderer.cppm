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
import mesh;
import ui;
import types;

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
    std::unique_ptr<CVulkanInstance> instance;
    std::unique_ptr<CVulkanDevice> device;
    std::unique_ptr<CVulkanSwapchain> swapchain;

    std::unique_ptr<CVulkanGraphicsPipeline> pipeline;

    std::unique_ptr<CVulkanCommandPool> graphicsCommandPool;
    std::unique_ptr<CVulkanCommandPool> computeCommandPool;
    std::unique_ptr<CVulkanCommandPool> transferCommandPool;

    std::unique_ptr<CVulkanUi> ui;

    std::unique_ptr<CVulkanMeshRenderer> meshRenderer;
    std::unique_ptr<CVulkanMeshLoader> meshLoader;
    std::vector<std::shared_ptr<CVulkanMesh>> meshes;

    std::vector<std::shared_ptr<CVulkanCommandBuffer>> graphicsCommandBuffers;
    std::vector<std::shared_ptr<CVulkanCommandBuffer>> computeCommandBuffers;
    std::vector<std::shared_ptr<CVulkanCommandBuffer>> transferCommandBuffers;

    std::unique_ptr<CVulkanQueue> graphicsQueue;
    std::unique_ptr<CVulkanQueue> computeQueue;
    std::unique_ptr<CVulkanQueue> transferQueue;
public:
    CVulkanRenderer(SDL_Window* window) {
        instance = std::make_unique<CVulkanInstance>(window);
        device = instance->CreateDevice();

        graphicsQueue = std::make_unique<CVulkanQueue>(device->GetGraphicsQueue());
        computeQueue = std::make_unique<CVulkanQueue>(device->GetComputeQueue());
        transferQueue = std::make_unique<CVulkanQueue>(device->GetTransferQueue());

        swapchain = std::make_unique<CVulkanSwapchain>(instance.get(), device.get(), graphicsQueue.get(), window, 2, true);

        graphicsCommandPool = std::make_unique<CVulkanCommandPool>(graphicsQueue->CreateCommandPool());
        computeCommandPool = std::make_unique<CVulkanCommandPool>(computeQueue->CreateCommandPool());
        transferCommandPool = std::make_unique<CVulkanCommandPool>(transferQueue->CreateCommandPool());
 
        for(int i = 0; i < swapchain->GetImageCount(); i++) {
            graphicsCommandBuffers.push_back(std::make_shared<CVulkanCommandBuffer>(graphicsCommandPool->CreateCommandBuffer()));
        }

        computeCommandBuffers.push_back(std::make_shared<CVulkanCommandBuffer>(computeCommandPool->CreateCommandBuffer()));
        transferCommandBuffers.push_back(std::make_shared<CVulkanCommandBuffer>(transferCommandPool->CreateCommandBuffer()));

        auto surfaceFormat = swapchain->GetVkSurfaceFormat();
        pipeline = std::make_unique<CVulkanGraphicsPipeline>(device->CreateGraphicsPipeline("vertex.spv", "fragment.spv", surfaceFormat));

        meshRenderer = std::make_unique<CVulkanMeshRenderer>(pipeline.get(), graphicsCommandBuffers);
        meshLoader = std::make_unique<CVulkanMeshLoader>(device.get(), transferQueue.get(), transferCommandPool.get(), transferCommandBuffers[0].get());
        meshes.push_back(std::make_shared<CVulkanMesh>(meshLoader->Load(vertices, indices)));
        ui = std::make_unique<CVulkanUi>(window, instance.get(), device.get(), graphicsQueue.get(), graphicsCommandBuffers[0].get(), 2, surfaceFormat);
    }

    void OnResize() {
        swapchain->Recreate();
    }

    void DrawFrame() {
        CVulkanFrame frame = swapchain->GetNextFrame();
        graphicsCommandPool->Reset();
        meshRenderer->Render(frame, meshes);
        graphicsQueue->Submit(graphicsCommandBuffers[frame.currentFrame].get(), frame.submitSemaphore, frame.acquireSemaphore, vk::PipelineStageFlagBits::eColorAttachmentOutput, frame.acquireFence);
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
};