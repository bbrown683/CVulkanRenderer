#include "renderer.hpp"

std::vector<CVulkanVertex> vertices = {
    CVulkanVertex(glm::vec2(0.0f, -0.5f), glm::vec3(1.0, .0f, 0.0f)),
    CVulkanVertex(glm::vec2(0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f)),
    CVulkanVertex(glm::vec2(-0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f)),
};

std::vector<uint16_t> indices = {
    0, 1, 2
};

CVulkanRenderer::CVulkanRenderer(CSDLWindow* window) {
    window->AddEventCallback(static_cast<void*>(this), SDL_EventFilterCallback); // Add callback when certain events fire.

    instance = std::make_unique<CVulkanInstance>(window->GetSDL_Window());
    device = instance->CreateDevice();
    graphicsQueue = device->GetGraphicsQueue();
    computeQueue = device->GetComputeQueue();
    transferQueue = device->GetTransferQueue();

    int imageCount = 2;
    swapchain = std::make_unique<CVulkanSwapchain>(instance.get(), device.get(), graphicsQueue.get(), window->GetSDL_Window(), imageCount, true);

    graphicsCommandPool = std::make_unique<CVulkanCommandPool>(graphicsQueue->CreateCommandPool());
    computeCommandPool = std::make_unique<CVulkanCommandPool>(computeQueue->CreateCommandPool());
    transferCommandPool = std::make_unique<CVulkanCommandPool>(transferQueue->CreateCommandPool());

    for(int i = 0; i < imageCount; i++) {
        graphicsCommandBuffers.push_back(std::make_shared<CVulkanCommandBuffer>(graphicsCommandPool->CreateCommandBuffer()));
    }

    computeCommandBuffer = std::make_shared<CVulkanCommandBuffer>(computeCommandPool->CreateCommandBuffer());
    transferCommandBuffer = std::make_shared<CVulkanCommandBuffer>(transferCommandPool->CreateCommandBuffer());

    auto surfaceFormat = swapchain->GetVkSurfaceFormat();
    pipeline = std::make_unique<CVulkanGraphicsPipeline>(device->CreateGraphicsPipeline("vertex.spv", "fragment.spv", surfaceFormat));

    meshRenderer = std::make_unique<CVulkanMeshRenderer>(pipeline.get(), graphicsCommandBuffers);
    meshLoader = std::make_unique<CVulkanMeshLoader>(device.get(), transferQueue.get(), transferCommandPool.get(), transferCommandBuffer);
    meshes.push_back(std::make_shared<CVulkanMesh>(meshLoader->Load(vertices, indices)));
    ui = std::make_unique<CVulkanUi>(window->GetSDL_Window(), instance.get(), device.get(), graphicsQueue.get(), graphicsCommandPool.get(), graphicsCommandBuffers, 2, surfaceFormat);
}

void CVulkanRenderer::OnResize() {
    swapchain->Recreate();
}

void CVulkanRenderer::DrawFrame() {
    CVulkanFrame frame = swapchain->GetNextFrame();
    graphicsCommandPool->Reset();
    CVulkanRender render;
    render.colorAttachments = { vk::RenderingAttachmentInfo(frame.imageView, vk::ImageLayout::eColorAttachmentOptimal,
                                                vk::ResolveModeFlagBits::eNone, nullptr, vk::ImageLayout::eUndefined,
                                                vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
                                                vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)) };

    auto currentCommandBuffer = graphicsCommandBuffers[frame.currentFrame];
    currentCommandBuffer->BeginPass(&frame, &render);
    meshRenderer->Draw(&frame, meshes);
#ifdef _DEBUG
    ui->Draw(&frame);
#endif
    currentCommandBuffer->EndPass(&frame);
    graphicsQueue->Submit(currentCommandBuffer, frame.submitSemaphore, frame.acquireSemaphore, vk::PipelineStageFlagBits::eColorAttachmentOutput, frame.acquireFence);
    swapchain->Present();
}

int CVulkanRenderer::SDL_EventFilterCallback(void* userdata, SDL_Event* event) {
    CVulkanRenderer* renderer = static_cast<CVulkanRenderer*>(userdata);
    if(renderer != nullptr) {
        if(event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED) {
            printf("CVulkanRenderer::OnWindowResizeSDL_EventCallback: Performing Resize\n");
            renderer->OnResize();
        }
    }
    return 0;
}
