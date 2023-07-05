module;
#include "platform/vulkan.hpp"
#include <glm/glm.hpp>
#include <fstream>
#include <vector>
export module pipeline;

// Vertex Properties.
export struct CVulkanVertex {
    glm::fvec4 position;
    glm::fvec3 normal;
    glm::fvec2 textureCoordinates;
    glm::fvec4 color;

    static std::vector<vk::VertexInputAttributeDescription> GetVkVertexInputAttributeDescriptions() {
        return { 
            vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(CVulkanVertex, position)),
            vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(CVulkanVertex, normal)),
            vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(CVulkanVertex, textureCoordinates)),
            vk::VertexInputAttributeDescription(3, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(CVulkanVertex, color)),
        };
    }

    static std::vector<vk::VertexInputBindingDescription> GetVkVertexInputBindingDecriptions() {
        return {
            vk::VertexInputBindingDescription(0, sizeof(CVulkanVertex), vk::VertexInputRate::eVertex)
        };
    }
};

// PBR Material Properties.
export struct CVulkanMaterial {
    float roughness;
    float metallic;
};

export struct CVulkanPushConstants {
    glm::mat4 modelViewProjection;
};

export class CVulkanGraphicsPipeline {
    vk::UniquePipeline pipeline;
    vk::UniquePipelineLayout layout;
    vk::UniqueShaderModule vertexShaderModule;
    vk::UniqueShaderModule fragmentShaderModule;
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
    std::vector<vk::VertexInputBindingDescription> bindingDescriptions;
public:
    CVulkanGraphicsPipeline() = default;
    CVulkanGraphicsPipeline(vk::Device device, vk::RenderPass renderPass, std::string vertexShaderFile, std::string fragmentShaderFile, vk::Extent2D extent) {
        std::vector<vk::PipelineShaderStageCreateInfo> shaderStagesInfo;

        // Vertex Shader
        std::vector<char> vertexShaderCode = ReadSPIRVFile(vertexShaderFile);
        vk::ShaderModuleCreateInfo vertexShaderModuleInfo;
        vertexShaderModuleInfo.codeSize = vertexShaderCode.size();
        vertexShaderModuleInfo.pCode = reinterpret_cast<uint32_t*>(vertexShaderCode.data());

        vertexShaderModule = device.createShaderModuleUnique(vertexShaderModuleInfo);
        shaderStagesInfo.push_back(vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, *vertexShaderModule, "main"));

        auto vertexInputAttributeDescriptions = CVulkanVertex::GetVkVertexInputAttributeDescriptions();
        auto vertexInputBindingDescriptions = CVulkanVertex::GetVkVertexInputBindingDecriptions();
        auto vertexInputStateInfo = vk::PipelineVertexInputStateCreateInfo({}, vertexInputBindingDescriptions, vertexInputAttributeDescriptions);

        // Fragment Shader
        std::vector<char> fragmentShaderCode = ReadSPIRVFile(fragmentShaderFile);
        vk::ShaderModuleCreateInfo fragmentShaderModuleInfo;
        fragmentShaderModuleInfo.codeSize = fragmentShaderCode.size();
        fragmentShaderModuleInfo.pCode = reinterpret_cast<uint32_t*>(fragmentShaderCode.data());

        fragmentShaderModule = device.createShaderModuleUnique(fragmentShaderModuleInfo);
        shaderStagesInfo.push_back(vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, *fragmentShaderModule, "main"));

        auto inputAssemblyStateInfo = vk::PipelineInputAssemblyStateCreateInfo({}, vk::PrimitiveTopology::eTriangleList);

        auto viewport = vk::Viewport(0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f);
        auto scissor = vk::Rect2D({ 0, 0 }, extent);
        auto viewportStateInfo = vk::PipelineViewportStateCreateInfo({}, viewport, scissor);

        auto rasterizationStateInfo = vk::PipelineRasterizationStateCreateInfo({}, false, false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone, vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f);
        auto multisampleStateInfo = vk::PipelineMultisampleStateCreateInfo({}, vk::SampleCountFlagBits::e1);
        auto colorBlendStateInfo = vk::PipelineColorBlendStateCreateInfo({});

        auto layoutInfo = vk::PipelineLayoutCreateInfo({});
        layout = device.createPipelineLayoutUnique(layoutInfo);

        auto pipelineInfo = vk::GraphicsPipelineCreateInfo({}, shaderStagesInfo, &vertexInputStateInfo, &inputAssemblyStateInfo, nullptr, &viewportStateInfo, &rasterizationStateInfo, &multisampleStateInfo, nullptr, &colorBlendStateInfo, nullptr, *layout, renderPass);
        vk::ResultValue<vk::UniquePipeline> createGraphicsPipelineResultValue = device.createGraphicsPipelineUnique(nullptr, pipelineInfo);
        vk::Result createGraphicsPipelineResult = createGraphicsPipelineResultValue.result;
        if(createGraphicsPipelineResult == vk::Result::eSuccess) {
            pipeline = std::move(createGraphicsPipelineResultValue.value);
        }
    }
    
    vk::ShaderModule GetVertexShaderModule() {
        return vk::ShaderModule();
    }

    vk::ShaderModule GetFragmentShaderModule() {
        return vk::ShaderModule();
    }

private:
    std::vector<char> ReadSPIRVFile(std::string filename) {
        std::ifstream inputStream(filename, std::ifstream::ate | std::ifstream::binary);
        if(!inputStream.is_open()) {
            printf("CVulkanPipeline::ReadFile: Failed to open %s", filename.c_str());
        }

        size_t fileLength = static_cast<size_t>(inputStream.tellg());
        std::vector<char> fileContent(fileLength);
        inputStream.seekg(0);
        inputStream.read(fileContent.data(), fileLength);
        inputStream.close();
        return fileContent;
    }
};

export class CVulkanComputePipeline {
    vk::UniquePipeline pipeline;
public:
    CVulkanComputePipeline() = default;
    CVulkanComputePipeline(vk::Device device, std::string computeShaderFile) {

    }
};