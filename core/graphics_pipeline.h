#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "vulkan_context.h"

class GraphicsPipelineBuilder {
public:
    GraphicsPipelineBuilder() = default;

    void AddShaderStage(VkShaderStageFlagBits stage, VkShaderModule module) {
        VkPipelineShaderStageCreateInfo stageInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = stage,
            .module = module,
            .pName = "main",
        };
        shader_stages_.push_back(stageInfo);
    }

    void SetVertexInput(
        const VkVertexInputBindingDescription* bindings, uint32_t bindingCount,
        const VkVertexInputAttributeDescription* attributes, uint32_t attributeCount) {
        vertex_input_info_ = VkPipelineVertexInputStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = bindingCount,
            .pVertexBindingDescriptions = bindings,
            .vertexAttributeDescriptionCount = attributeCount,
            .pVertexAttributeDescriptions = attributes,
        };
        bindings_.assign(bindings, bindings + bindingCount);
        attributes_.assign(attributes, attributes + attributeCount);
    }

    void SetViewport(const VkViewport& viewport, const VkRect2D& scissor) {
        viewport_ = viewport;
        scissor_ = scissor;
    }

    void SetPipelineLayout(VkPipelineLayout layout) {
        pipeline_layout_ = layout;
    }

    void UseDynamicRendering(VkFormat colorFormat) {
        color_format_ = colorFormat;
        use_dynamic_rendering_ = true;
    }

    VkPipeline Build() {
        auto& context = VulkanContext::Get();
        VkDevice device = context.GetDevice();

        vertex_input_info_.pVertexBindingDescriptions = bindings_.data();
        vertex_input_info_.pVertexAttributeDescriptions = attributes_.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE,
        };

        VkPipelineViewportStateCreateInfo viewportState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports = &viewport_,
            .scissorCount = 1,
            .pScissors = &scissor_,
        };

        VkPipelineRasterizationStateCreateInfo rasterizer{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .lineWidth = 1.0f,
        };

        VkPipelineMultisampleStateCreateInfo multisampling{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
        };

        VkPipelineColorBlendAttachmentState colorBlendAttachment{
            .blendEnable = VK_FALSE,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        };

        VkPipelineColorBlendStateCreateInfo colorBlending{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachment,
        };

        VkGraphicsPipelineCreateInfo pipelineInfo{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .stageCount = static_cast<uint32_t>(shader_stages_.size()),
            .pStages = shader_stages_.data(),
            .pVertexInputState = &vertex_input_info_,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pColorBlendState = &colorBlending,
            .layout = pipeline_layout_,
            .renderPass = VK_NULL_HANDLE,
            .subpass = 0,
        };

        VkPipelineRenderingCreateInfo renderingInfo{};
        if (use_dynamic_rendering_) {
            renderingInfo = VkPipelineRenderingCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                .colorAttachmentCount = 1,
                .pColorAttachmentFormats = &color_format_,
            };
            pipelineInfo.pNext = &renderingInfo;
        }

        VkPipeline pipeline;
        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        return pipeline;
    }

private:
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages_;
    std::vector<VkVertexInputBindingDescription> bindings_;
    std::vector<VkVertexInputAttributeDescription> attributes_;
    VkPipelineVertexInputStateCreateInfo vertex_input_info_{};
    VkViewport viewport_{};
    VkRect2D scissor_{};
    VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;
    VkFormat color_format_ = VK_FORMAT_UNDEFINED;
    bool use_dynamic_rendering_ = false;
};
