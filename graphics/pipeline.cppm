module;
#include "platform/vulkan.hpp"
export module pipeline;

export class CVulkanPipeline {
    vk::UniquePipeline pipeline;
    vk::UniquePipelineLayout layout;
public:
    CVulkanPipeline(vk::RenderPass renderPass, vk::Extent2D extent) {

    }
};