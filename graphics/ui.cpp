#include "ui.hpp"
#include <SDL2/SDL.h>

#define IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_vulkan.h>

#include "instance.hpp"
#include "device.hpp"
#include "queue.hpp"
#include "cmd.hpp"
#include "image.hpp"
#include "types.hpp"

CVulkanUi::CVulkanUi(SDL_Window* window, CVulkanInstance* instance, CVulkanDevice* device, 
    CVulkanQueue* queue, CVulkanCommandPool* commandPool, std::vector<std::shared_ptr<CVulkanCommandBuffer>> commandBuffers, 
    uint32_t imageCount, vk::Format colorFormat)
    : window(window), instance(instance), device(device), queue(queue), commandPool(commandPool), commandBuffers(commandBuffers) {
    auto vkInstance = instance->GetVkInstance();
    auto vkPhysicalDevice = device->GetVkPhysicalDevice();
    auto vkDevice = device->GetVkDevice();
    auto vkQueue = queue->GetVkQueue();
    auto queueFamily = queue->GetFamilyIndex();

    std::vector<vk::DescriptorPoolSize> descriptorPoolSizes = {
        { vk::DescriptorType::eCombinedImageSampler, 1 },
    };

    vk::DescriptorPoolCreateInfo descriptorPoolInfo;
    descriptorPoolInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
    descriptorPoolInfo.setMaxSets(1);
    descriptorPoolInfo.setPoolSizes(descriptorPoolSizes);

    descriptorPool = std::make_unique<vk::raii::DescriptorPool>(vkDevice->createDescriptorPool(descriptorPoolInfo));

    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

    io.Fonts->AddFontFromFileTTF("fonts/Roboto-Bold.ttf", 16.0f);

    vk::Format colorAttachmentFormat = vk::Format::eB8G8R8A8Srgb;
    vk::PipelineRenderingCreateInfoKHR pipelineInfo;
    pipelineInfo.pColorAttachmentFormats = &colorAttachmentFormat;
    pipelineInfo.colorAttachmentCount = 1;

    ImGui_ImplVulkan_InitInfo imguiVulkanInitInfo = {};
    imguiVulkanInitInfo.Instance = **vkInstance;
    imguiVulkanInitInfo.PhysicalDevice = vkPhysicalDevice;
    imguiVulkanInitInfo.Device = **vkDevice;
    imguiVulkanInitInfo.Queue = **vkQueue;
    imguiVulkanInitInfo.QueueFamily = queueFamily;
    imguiVulkanInitInfo.PipelineCache = nullptr;
    imguiVulkanInitInfo.DescriptorPool = **descriptorPool;
    imguiVulkanInitInfo.Subpass = 0;
    imguiVulkanInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    imguiVulkanInitInfo.MinImageCount = imageCount;
    imguiVulkanInitInfo.ImageCount = imageCount;
    imguiVulkanInitInfo.CheckVkResultFn = nullptr;
    imguiVulkanInitInfo.Allocator = nullptr;
    imguiVulkanInitInfo.UseDynamicRendering = true;
    imguiVulkanInitInfo.PipelineRenderingCreateInfo = pipelineInfo;
    //imguiVulkanInitInfo.ColorAttachmentFormat = VK_FORMAT_B8G8R8A8_SRGB;

    //if (!ImGui_ImplSDL2_InitForVulkan(window) || !ImGui_ImplVulkan_Init(&imguiVulkanInitInfo, nullptr)) {
    if(!ImGui_ImplSDL2_InitForVulkan(window) || !ImGui_ImplVulkan_Init(&imguiVulkanInitInfo)) {
        printf("CVulkanUi::CVulkanUi: Failed to initialize ImGui");
    }
    //ImGui_ImplVulkan_CreateFontsTexture();
    
    // Doesnt appear to be needed anymore, has its own implementation.
    //commandBuffers[0]->UploadImguiFonts();
    //queue->Submit(commandBuffers[0]);
    //ImGui_ImplVulkan_DestroyFontsTexture();
    //commandPool->Reset();
}

CVulkanUi::~CVulkanUi() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void CVulkanUi::Draw(CVulkanFrame* frame) {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if(ImGui::BeginMainMenuBar()) {
        if(ImGui::BeginMenu("File")) {
            ImGui::MenuItem("New");
            ImGui::MenuItem("Open");
            ImGui::Separator();
            ImGui::MenuItem("Save");
            ImGui::MenuItem("Save As");
            ImGui::Separator();
            ImGui::MenuItem("Import");
            ImGui::MenuItem("Export");
            ImGui::Separator();
            ImGui::MenuItem("Quit");
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Edit")) {
            ImGui::MenuItem("Undo");
            ImGui::MenuItem("Redo");
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    if(ImGui::Begin("Renderer")) {
        ImVec2 viewport = ImGui::GetContentRegionAvail();
        viewportExtent = vk::Extent2D{ static_cast<uint32_t>(viewport.x), static_cast<uint32_t>(viewport.y) };
        ImGui::End();
    }

    if(ImGui::Begin("Scene")) {
        ImGui::End();
    }

    if(ImGui::Begin("Properties")) {
        ImGui::End();
    }

    bool showDemoWindow = true;
    ImGui::ShowDemoWindow(&showDemoWindow);
    ImGui::Render();
    if(frame != nullptr) {
        ImDrawData* drawData = ImGui::GetDrawData();
        commandBuffers[frame->currentFrame]->Draw(drawData);
    }
}