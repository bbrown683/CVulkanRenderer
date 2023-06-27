module;
#include "platform/vulkan.hpp"
module shader;
import <vector>;
import device;

CVulkanShader::CVulkanShader(vk::Device device) : device(device) {
    
}

vk::ShaderModule CVulkanShader::GetVertexShaderModule() {
    return vk::ShaderModule();
}

vk::ShaderModule CVulkanShader::GetFragmentShaderModule() {
    return vk::ShaderModule();
}
