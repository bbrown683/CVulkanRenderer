#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

class CVulkanGraphicsPipeline {
    std::unique_ptr<vk::raii::Pipeline> pipeline;
    std::unique_ptr<vk::raii::PipelineLayout> layout;
    std::unique_ptr<vk::raii::DescriptorSet> descriptorSet;
    std::unique_ptr<vk::raii::DescriptorSetLayout> descriptorSetLayout;
public:
    CVulkanGraphicsPipeline(std::shared_ptr<vk::raii::Device> device, std::string vertexShaderFile, std::string fragmentShaderFile, vk::Format colorFormat);
    vk::Pipeline GetVkPipeline();
private:
    std::vector<char> ReadSPIRVFile(std::string filename);
};

class CVulkanComputePipeline {
    std::shared_ptr<vk::raii::Pipeline> pipeline;
public:
    CVulkanComputePipeline() = default;
    CVulkanComputePipeline(std::shared_ptr<vk::raii::Device> device, std::string computeShaderFile) {

    }
};