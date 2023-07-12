module;
#include "platform/vulkan.hpp"
#include <glm/glm.hpp>
#include <fstream>
#include <vector>
export module pipeline;

// Vertex Properties.
export struct CVulkanVertex {
    glm::vec2 position;
    glm::vec3 color;

    static std::vector<vk::VertexInputAttributeDescription> GetVkVertexInputAttributeDescriptions() {
        return { 
            vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32Sfloat, offsetof(CVulkanVertex, position)),
            vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(CVulkanVertex, color)),
        };
    }

    static vk::VertexInputBindingDescription GetVkVertexInputBindingDecription() {
        return vk::VertexInputBindingDescription(0, sizeof(CVulkanVertex), vk::VertexInputRate::eVertex);
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
    vk::UniqueDescriptorSet descriptorSet;
    vk::UniqueDescriptorSetLayout descriptorSetLayout;
public:
    CVulkanGraphicsPipeline() = default;
    CVulkanGraphicsPipeline(vk::Device device, std::string vertexShaderFile, std::string fragmentShaderFile, vk::Format colorFormat) {
        std::vector<vk::PipelineShaderStageCreateInfo> shaderStagesInfo;

        // Vertex Shader
        std::vector<char> vertexShaderCode = ReadSPIRVFile(vertexShaderFile);
        vk::ShaderModuleCreateInfo vertexShaderModuleInfo;
        vertexShaderModuleInfo.codeSize = vertexShaderCode.size();
        vertexShaderModuleInfo.pCode = reinterpret_cast<uint32_t*>(vertexShaderCode.data());

        vk::UniqueShaderModule vertexShaderModule = device.createShaderModuleUnique(vertexShaderModuleInfo);

        vk::PipelineShaderStageCreateInfo vertexShaderStageInfo;
        vertexShaderStageInfo.setStage(vk::ShaderStageFlagBits::eVertex);
        vertexShaderStageInfo.setModule(*vertexShaderModule);
        vertexShaderStageInfo.setPName("main");
        shaderStagesInfo.push_back(vertexShaderStageInfo);

        auto vertexInputAttributeDescriptions = CVulkanVertex::GetVkVertexInputAttributeDescriptions();
        auto vertexInputBindingDescription = CVulkanVertex::GetVkVertexInputBindingDecription();
        vk::PipelineVertexInputStateCreateInfo vertexInputStateInfo({}, vertexInputBindingDescription, vertexInputAttributeDescriptions);

        // Fragment Shader
        std::vector<char> fragmentShaderCode = ReadSPIRVFile(fragmentShaderFile);
        vk::ShaderModuleCreateInfo fragmentShaderModuleInfo;
        fragmentShaderModuleInfo.codeSize = fragmentShaderCode.size();
        fragmentShaderModuleInfo.pCode = reinterpret_cast<uint32_t*>(fragmentShaderCode.data());

        vk::UniqueShaderModule fragmentShaderModule = device.createShaderModuleUnique(fragmentShaderModuleInfo);

        vk::PipelineShaderStageCreateInfo fragmentShaderStageInfo;
        fragmentShaderStageInfo.setStage(vk::ShaderStageFlagBits::eFragment);
        fragmentShaderStageInfo.setModule(*fragmentShaderModule);
        fragmentShaderStageInfo.setPName("main");
        shaderStagesInfo.push_back(fragmentShaderStageInfo);

        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo; 
        inputAssemblyStateInfo.setTopology(vk::PrimitiveTopology::eTriangleList);

        vk::Viewport viewport;
        vk::Rect2D scissor;
        vk::PipelineViewportStateCreateInfo viewportStateInfo({}, viewport, scissor);

        vk::PipelineRasterizationStateCreateInfo rasterizationStateInfo;
        rasterizationStateInfo.setPolygonMode(vk::PolygonMode::eFill);
        rasterizationStateInfo.setCullMode(vk::CullModeFlagBits::eNone);
        rasterizationStateInfo.setFrontFace(vk::FrontFace::eCounterClockwise);
        rasterizationStateInfo.setLineWidth(1.0f);

        vk::PipelineMultisampleStateCreateInfo multisampleStateInfo;
        multisampleStateInfo.setRasterizationSamples(vk::SampleCountFlagBits::e1);

        vk::PipelineColorBlendAttachmentState colorBlendAttachmentState;
        colorBlendAttachmentState.setBlendEnable(false);
        colorBlendAttachmentState.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eA);

        vk::PipelineColorBlendStateCreateInfo colorBlendStateInfo;
        colorBlendStateInfo.setAttachments(colorBlendAttachmentState);

        // These will be modified via command buffers dynamically instead.
        auto dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
        vk::PipelineDynamicStateCreateInfo dynamicStateInfo({}, dynamicStates);

        /*
        auto descriptorSetLayoutInfo = vk::DescriptorSetLayoutCreateInfo({});
        descriptorSetLayout = device.createDescriptorSetLayoutUnique(descriptorSetLayoutInfo);

        auto pushConstantRange = vk::PushConstantRange(vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4));
        */
        auto layoutInfo = vk::PipelineLayoutCreateInfo({});
        layout = device.createPipelineLayoutUnique(layoutInfo);

        /*
        // Provide information for dynamic rendering
        VkPipelineRenderingCreateInfoKHR pipeline_create{VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR};
        pipeline_create.pNext                   = VK_NULL_HANDLE;
        pipeline_create.colorAttachmentCount    = 1;
        pipeline_create.pColorAttachmentFormats = &color_rendering_format;
        pipeline_create.depthAttachmentFormat   = depth_format;
        pipeline_create.stencilAttachmentFormat = depth_format;
        */
        vk::PipelineRenderingCreateInfo pipelineRenderingInfo;
        pipelineRenderingInfo.setColorAttachmentFormats(colorFormat);

        vk::GraphicsPipelineCreateInfo pipelineInfo;
        pipelineInfo.setStages(shaderStagesInfo);
        pipelineInfo.setPVertexInputState(&vertexInputStateInfo);
        pipelineInfo.setPInputAssemblyState(&inputAssemblyStateInfo);
        pipelineInfo.setPViewportState(&viewportStateInfo);
        pipelineInfo.setPRasterizationState(&rasterizationStateInfo);
        pipelineInfo.setPMultisampleState(&multisampleStateInfo);
        pipelineInfo.setPColorBlendState(&colorBlendStateInfo);
        pipelineInfo.setPDynamicState(&dynamicStateInfo);
        pipelineInfo.setLayout(*layout);
        pipelineInfo.setPNext(&pipelineRenderingInfo);
        
        vk::ResultValue<vk::UniquePipeline> createGraphicsPipelineResultValue = device.createGraphicsPipelineUnique(nullptr, pipelineInfo);
        vk::Result createGraphicsPipelineResult = createGraphicsPipelineResultValue.result;
        if(createGraphicsPipelineResult == vk::Result::eSuccess) {
            pipeline = std::move(createGraphicsPipelineResultValue.value);
        }
    }
    
    vk::Pipeline GetVkPipeline() {
        return *pipeline;
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