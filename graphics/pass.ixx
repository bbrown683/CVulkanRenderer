module;
#include "platform/vulkan.hpp"
export module pass;
import <vector>;
import <optional>;

export class CVulkanRenderPass {
    vk::Device device;
    vk::UniqueRenderPass renderPass;
public:
    CVulkanRenderPass(vk::Device device, vk::Format colorFormat, std::optional<vk::Format> depthStencilFormat = std::nullopt, std::optional<vk::ClearColorValue> clearColor = std::nullopt);
};