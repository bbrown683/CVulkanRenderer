module;
#include "platform/SDL.hpp"
#include "platform/vulkan.hpp"
#include "platform/imgui/imgui.h"
#include "platform/imgui/imgui_impl_sdl2.h"
#include "platform/imgui/imgui_impl_vulkan.h"
export module ui;
import instance;
import device;
import queue;
import pipeline;
import cmd;
import types;

export class CVulkanUi {
    vk::UniqueDescriptorPool descriptorPool;
    vk::UniquePipelineCache pipelineCache;
    std::vector<std::shared_ptr<CVulkanCommandBuffer>> commandBuffers;
public:
    CVulkanUi(SDL_Window* window, CVulkanInstance* instance, CVulkanDevice* device, CVulkanQueue* queue, 
              CVulkanCommandPool* commandPool, std::vector<std::shared_ptr<CVulkanCommandBuffer>> commandBuffers, 
              uint32_t imageCount, vk::Format colorFormat)
            : commandBuffers(commandBuffers) {
        auto vkInstance = instance->GetVkInstance();
        auto vkPhysicalDevice = device->GetVkPhysicalDevice();
        auto vkDevice = device->GetVkDevice();
        auto vkQueue = queue->GetVkQueue();
        auto queueFamily = queue->GetFamilyIndex();

        std::vector<vk::DescriptorPoolSize> descriptorPoolSizes = {
            { vk::DescriptorType::eSampler, 1000 },
            { vk::DescriptorType::eCombinedImageSampler, 1000 },
            { vk::DescriptorType::eSampledImage, 1000 },
            { vk::DescriptorType::eStorageImage, 1000 },
            { vk::DescriptorType::eUniformTexelBuffer, 1000 },
            { vk::DescriptorType::eStorageTexelBuffer, 1000 },
            { vk::DescriptorType::eUniformBuffer, 1000 },
            { vk::DescriptorType::eStorageBuffer, 1000 },
            { vk::DescriptorType::eUniformBufferDynamic, 1000 },
            { vk::DescriptorType::eStorageBufferDynamic, 1000 },
            { vk::DescriptorType::eInputAttachment, 1000 },
        };

        vk::DescriptorPoolCreateInfo descriptorPoolInfo;
        descriptorPoolInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
        descriptorPoolInfo.setMaxSets(1000);
        descriptorPoolInfo.setPoolSizes(descriptorPoolSizes);

        descriptorPool = vkDevice.createDescriptorPoolUnique(descriptorPoolInfo);

        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

        ImGui_ImplVulkan_InitInfo imguiVulkanInitInfo;
        imguiVulkanInitInfo.Instance = vkInstance;
        imguiVulkanInitInfo.PhysicalDevice = vkPhysicalDevice;
        imguiVulkanInitInfo.Device = vkDevice;
        imguiVulkanInitInfo.Queue = vkQueue;
        imguiVulkanInitInfo.QueueFamily = queueFamily;
        imguiVulkanInitInfo.PipelineCache = *pipelineCache;
        imguiVulkanInitInfo.DescriptorPool = *descriptorPool;
        imguiVulkanInitInfo.Subpass = 0;
        imguiVulkanInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        imguiVulkanInitInfo.MinImageCount = imageCount;
        imguiVulkanInitInfo.ImageCount = imageCount;
        imguiVulkanInitInfo.CheckVkResultFn = nullptr;
        imguiVulkanInitInfo.Allocator = nullptr;
        imguiVulkanInitInfo.UseDynamicRendering = true;
        imguiVulkanInitInfo.ColorAttachmentFormat = VK_FORMAT_B8G8R8A8_UNORM;

        if(!ImGui_ImplSDL2_InitForVulkan(window) || !ImGui_ImplVulkan_Init(&imguiVulkanInitInfo, nullptr)) {
            printf("CVulkanUi::CVulkanUi: Failed to initialize ImGui");
        }

        commandBuffers[0]->UploadImguiFonts();
        queue->Submit(commandBuffers[0]);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
        commandPool->Reset();
    }

    ~CVulkanUi() {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }

    void Draw(CVulkanFrame frame) {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        bool showDemoWindow = true;
        ImGui::ShowDemoWindow(&showDemoWindow);
        ImGui::Render();
        ImDrawData* drawData = ImGui::GetDrawData();
        commandBuffers[frame.currentFrame]->Draw(drawData);

        ImGuiIO& io = ImGui::GetIO();
        // Update and Render additional Platform Windows
        if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }
};