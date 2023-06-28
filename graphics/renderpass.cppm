module;
#include "platform/vulkan.hpp"
export module renderpass;
import <vector>;

export class CVulkanRenderPass {
    vk::UniqueRenderPass renderPass;
    vk::ClearColorValue clearColor;
public:
    CVulkanRenderPass() = default;
    CVulkanRenderPass(vk::Device device, vk::Format colorFormat, vk::Format depthStencilFormat = vk::Format::eUndefined, vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1, vk::ClearColorValue clearColor = vk::ClearColorValue())
        : clearColor(clearColor) {
        std::vector<vk::AttachmentDescription> colorAttachments = { vk::AttachmentDescription(vk::AttachmentDescriptionFlags(), colorFormat, samples,
                                  vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
                                  vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                                  vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR) };
        auto colorReferences = vk::AttachmentReference(static_cast<uint32_t>(colorAttachments.size()) - 1, vk::ImageLayout::eColorAttachmentOptimal);

        std::vector<vk::SubpassDescription> subpasses;
        if(depthStencilFormat != vk::Format::eUndefined) {
            auto depthStencilAttachment = vk::AttachmentDescription(vk::AttachmentDescriptionFlags(), depthStencilFormat, samples,
                                                                    vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
                                                                    vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                                                                    vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);
            auto depthStencilReference = vk::AttachmentReference(static_cast<uint32_t>(colorAttachments.size()), vk::ImageLayout::eDepthAttachmentOptimal);
            subpasses.push_back(vk::SubpassDescription(vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics, nullptr, colorReferences, nullptr, &depthStencilReference));
        } else {
            subpasses.push_back(vk::SubpassDescription(vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics, nullptr, colorReferences));
        }

        auto subpassDependency = vk::SubpassDependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlags(), vk::AccessFlagBits::eColorAttachmentWrite);
        auto renderPassInfo = vk::RenderPassCreateInfo(vk::RenderPassCreateFlags(), colorAttachments, subpasses, subpassDependency);
        renderPass = device.createRenderPassUnique(renderPassInfo);
    }

    vk::RenderPass GetVkRenderPass() {
        return *renderPass;
    }

    vk::ClearColorValue GetVkClearColorValue() {
        return clearColor;
    }
};