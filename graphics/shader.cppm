module;
#include "platform/vulkan.hpp"
#include <glm/glm.hpp>
export module shader;
import <vector>;

export struct CVulkanVertex {
    glm::fvec4 position;
    glm::fvec3 normal;
    glm::fvec2 textureCoordinates;
    glm::fvec4 color;

    static std::vector<vk::VertexInputAttributeDescription> GetVkVertexInputAttributeDescription() {
        return { 
            vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(CVulkanVertex, position)),
            vk::VertexInputAttributeDescription(0, 1, vk::Format::eR32G32B32Sfloat, offsetof(CVulkanVertex, normal)),
            vk::VertexInputAttributeDescription(0, 2, vk::Format::eR32G32Sfloat, offsetof(CVulkanVertex, textureCoordinates)),
            vk::VertexInputAttributeDescription(0, 3, vk::Format::eR32G32B32A32Sfloat, offsetof(CVulkanVertex, color)),
        };
    }

    static std::vector<vk::VertexInputBindingDescription> GetVkVertexInputBindingDecription() {
        return {
            vk::VertexInputBindingDescription(0, sizeof(CVulkanVertex), vk::VertexInputRate::eVertex)
        };
    }
};

export struct CVulkanMaterial {
    vk::UniqueSampler textureSampler;

};

export struct CVulkanPushConstants {
    glm::mat4 modelViewProjection;
};

export class CVulkanPipeline {
    vk::UniqueShaderModule vertexShaderModule;
    vk::UniqueShaderModule fragmentShaderModule;
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
    std::vector<vk::VertexInputBindingDescription> bindingDescriptions;
public:
    CVulkanPipeline() = default;
    CVulkanPipeline(vk::Device device, std::string vertexShaderCode, std::string fragmentShaderCode) {
        auto vertexShaderModuleInfo = vk::ShaderModuleCreateInfo(vk::ShaderModuleCreateFlags());
    }
    
    vk::ShaderModule GetVertexShaderModule() {
        return vk::ShaderModule();
    }

    vk::ShaderModule GetFragmentShaderModule() {
        return vk::ShaderModule();
    }

private:

};