#pragma once
#include <memory>
#include <vector>

#include <SDL2/SDL.h>

#include "window.hpp"
#include "instance.hpp"
#include "device.hpp"
#include "queue.hpp"
#include "swapchain.hpp"
#include "cmd.hpp"
#include "buffer.hpp"
#include "pipeline.hpp"
#include "mesh.hpp"
#include "ui.hpp"
#include "types.hpp"

class CVulkanRenderer {
    std::unique_ptr<CVulkanInstance> instance;
    std::unique_ptr<CVulkanDevice> device;
    std::unique_ptr<CVulkanSwapchain> swapchain;

    std::unique_ptr<CVulkanGraphicsPipeline> pipeline;

    std::unique_ptr<CVulkanCommandPool> graphicsCommandPool;
    std::unique_ptr<CVulkanCommandPool> computeCommandPool;
    std::unique_ptr<CVulkanCommandPool> transferCommandPool;

    std::vector<std::shared_ptr<CVulkanCommandBuffer>> graphicsCommandBuffers;
    std::shared_ptr<CVulkanCommandBuffer> computeCommandBuffer;
    std::shared_ptr<CVulkanCommandBuffer> transferCommandBuffer;

    std::unique_ptr<CVulkanUi> ui;

    std::unique_ptr<CVulkanMeshRenderer> meshRenderer;
    std::unique_ptr<CVulkanMeshLoader> meshLoader;
    std::vector<std::shared_ptr<CVulkanMesh>> meshes;

    std::unique_ptr<CVulkanQueue> graphicsQueue;
    std::unique_ptr<CVulkanQueue> computeQueue;
    std::unique_ptr<CVulkanQueue> transferQueue;
public:
    CVulkanRenderer(CSDLWindow* window);
    void OnResize();
    void DrawFrame();
    // Hook up events to the renderer.
    static int SDL_EventFilterCallback(void* userdata, SDL_Event* event);
};