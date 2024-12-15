#include "pipeline.hpp"

#include <fstream>
#include "types.hpp"

CVulkanGraphicsPipeline::CVulkanGraphicsPipeline(std::shared_ptr<vk::raii::Device> device, std::string vertexShaderFile, std::string fragmentShaderFile, vk::Format colorFormat) {
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStagesInfo;

    // Vertex Shader
    std::vector<char> vertexShaderCode = ReadSPIRVFile(vertexShaderFile);
    vk::ShaderModuleCreateInfo vertexShaderModuleInfo;
    vertexShaderModuleInfo.codeSize = vertexShaderCode.size();
    vertexShaderModuleInfo.pCode = reinterpret_cast<uint32_t*>(vertexShaderCode.data());

    vk::raii::ShaderModule vertexShaderModule = vk::raii::ShaderModule(*device, vertexShaderModuleInfo);

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

    vk::raii::ShaderModule fragmentShaderModule = vk::raii::ShaderModule(*device, fragmentShaderModuleInfo);

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
    layout = std::make_unique<vk::raii::PipelineLayout>(*device, layoutInfo);

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
    pipelineInfo.setLayout(**layout);
    pipelineInfo.setPNext(&pipelineRenderingInfo);
    pipeline = std::make_unique<vk::raii::Pipeline>(*device, nullptr, pipelineInfo);
}

vk::Pipeline CVulkanGraphicsPipeline::GetVkPipeline() {
    return **pipeline;
}

std::vector<char> CVulkanGraphicsPipeline::ReadSPIRVFile(std::string filename) {
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
