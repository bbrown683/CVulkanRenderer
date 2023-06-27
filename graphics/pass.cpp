module;
#include "platform/vulkan.hpp"
module pass;
import <vector>;
import <optional>;

CVulkanRenderPass::CVulkanRenderPass(vk::Device device, vk::Format colorFormat, std::optional<vk::Format> depthStencilFormat, std::optional<vk::ClearColorValue> clearColor) : device(device) {
}