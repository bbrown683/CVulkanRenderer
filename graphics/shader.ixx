module;
#include "platform/vulkan.hpp"
export module shader;
import <vector>;

export class CVulkanShader {
    vk::Device device;
    vk::UniqueShaderModule vertexShaderModule;
    vk::UniqueShaderModule fragmentShaderModule;
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
    std::vector<vk::VertexInputBindingDescription> bindingDescriptions;
public:
    CVulkanShader(vk::Device device);
    vk::ShaderModule GetVertexShaderModule();
    vk::ShaderModule GetFragmentShaderModule();
};