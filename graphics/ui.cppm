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
import cmd;

export class CVulkanUi {
    vk::UniqueDescriptorPool descriptorPool;
    vk::UniquePipelineCache pipelineCache;
public:
    CVulkanUi() = default;
    CVulkanUi(SDL_Window* window, CVulkanInstance* instance, CVulkanDevice* device, CVulkanQueue* queue, CVulkanCommandBuffer* commandBuffer, uint32_t imageCount, vk::Format colorFormat) {
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


        auto vkDevice = device->GetVkDevice();
        auto vkPhysicalDevice = device->GetVkPhysicalDevice();
        descriptorPool = vkDevice.createDescriptorPoolUnique(descriptorPoolInfo);

        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

        auto vkInstance = instance->GetVkInstance();
        auto vkQueue = queue->GetVkQueue();
        auto queueFamily = queue->GetFamilyIndex();

        ImGui_ImplVulkan_InitInfo imguiVulkanInitInfo;
        imguiVulkanInitInfo.Instance = vkInstance;
        imguiVulkanInitInfo.PhysicalDevice = vkPhysicalDevice;
        imguiVulkanInitInfo.Device = vkDevice;
        imguiVulkanInitInfo.Queue = vkQueue;
        imguiVulkanInitInfo.PipelineCache = *pipelineCache;
        imguiVulkanInitInfo.DescriptorPool = *descriptorPool;
        imguiVulkanInitInfo.Subpass = 0;
        imguiVulkanInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        imguiVulkanInitInfo.MinImageCount = imageCount;
        imguiVulkanInitInfo.ImageCount = imageCount;
        imguiVulkanInitInfo.CheckVkResultFn = nullptr;
        imguiVulkanInitInfo.Allocator = nullptr;
        imguiVulkanInitInfo.UseDynamicRendering = true;
        imguiVulkanInitInfo.ColorAttachmentFormat = static_cast<VkFormat>(colorFormat);

        if(!ImGui_ImplSDL2_InitForVulkan(window) || !ImGui_ImplVulkan_Init(&imguiVulkanInitInfo, nullptr)) {
            printf("CVulkanUi::CVulkanUi: Failed to initialize ImGui");
        }

        commandBuffer->Begin();
        auto vkCommandBuffer = commandBuffer->GetVkCommandBuffer();
        ImGui_ImplVulkan_CreateFontsTexture(vkCommandBuffer);
        commandBuffer->End();
        queue->Submit(vkCommandBuffer);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    ~CVulkanUi() {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL2_Shutdown();
    }

    ImDrawData* GetState() {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        bool showDemoWindow = true;
        ImGui::ShowDemoWindow(&showDemoWindow);

        ImGui::Render();
        return ImGui::GetDrawData();
    }
};