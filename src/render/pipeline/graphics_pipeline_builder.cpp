#include "render/pipeline/graphics_pipeline_builder.h"
#include "render/vulkan_context.h"

using namespace trinity::render;




GraphicsPipelineBuilder::GraphicsPipelineBuilder()
{
    vertex_input_state_ =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    };

    input_assembly_state_ =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    rasterizer_state_ =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f,
    };

    multisample_state_ =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
    };

    color_blend_attachment_ =
    {
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};

    color_blend_state_ =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment_,
    };

    depth_stencil_state_ = VkPipelineDepthStencilStateCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_FALSE,
        .depthWriteEnable = VK_FALSE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE
    };
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddShaderStage(VkShaderStageFlagBits stage, VkShaderModule module, const char* entry)
{
    shader_stages_.push_back(
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = stage,
        .module = module,
        .pName = entry
    });
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::SetVertexInput(
        const VkVertexInputBindingDescription *bindings, uint32_t bindingCount,
        const VkVertexInputAttributeDescription *attributes,
        uint32_t attributeCount)
{
    binding_descriptions_.assign(bindings, bindings + bindingCount);
    attribute_descriptions_.assign(attributes, attributes + attributeCount);

    vertex_input_state_ =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = bindingCount,
        .pVertexBindingDescriptions = binding_descriptions_.data(),
        .vertexAttributeDescriptionCount = attributeCount,
        .pVertexAttributeDescriptions = attribute_descriptions_.data()
    };
    return *this;
}

GraphicsPipelineBuilder&
GraphicsPipelineBuilder::SetViewport(VkExtent2D extent)
{
    viewport_ =
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = float(extent.width),
        .height = float(extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    // Y-flip using VK_KHR_Maintenance1
    viewport_.y = float(extent.height);
    viewport_.height = -float(extent.height);

    scissor_ = {.offset = {0, 0}, .extent = extent};

    viewport_state_ =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport_,
        .scissorCount = 1,
        .pScissors = &scissor_
    };

    return *this;
}

GraphicsPipelineBuilder&
GraphicsPipelineBuilder::SetViewport(VkViewport &viewport, VkRect2D scissor)
{
    viewport_ = viewport;
    scissor_ = scissor;
    viewport_state_ =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport_,
        .scissorCount = 1,
        .pScissors = &scissor_
    };
    return *this;
}

void GraphicsPipelineBuilder::SetColorBlendAttachmentState(
        const VkPipelineColorBlendAttachmentState &state)
{
    color_blend_attachment_ = state;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::EnableAlphaBlend()
{
    // Premultiplied Alpha Blending
    color_blend_attachment_.blendEnable = VK_TRUE;
    color_blend_attachment_.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment_.dstColorBlendFactor =
            VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment_.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment_.dstAlphaBlendFactor =
            VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_.alphaBlendOp = VK_BLEND_OP_ADD;

    return *this;
}

void GraphicsPipelineBuilder::SetRasterizationState(
        const VkPipelineRasterizationStateCreateInfo &state)
{
    rasterizer_state_ = state;
}

void GraphicsPipelineBuilder::SetDepthStencilState(
        const VkPipelineDepthStencilStateCreateInfo &state)
{
    depth_stencil_state_ = state;
}

GraphicsPipelineBuilder&
GraphicsPipelineBuilder::SetPipelineLayout(VkPipelineLayout layout)
{
    pipeline_layout_ = layout;
    return *this;
}

GraphicsPipelineBuilder&
GraphicsPipelineBuilder::UseRenderPass(VkRenderPass renderPass, uint32_t subpass)
{
    use_render_pass_ = true;
    render_pass_ = renderPass;
    subpass_ = subpass;
    return *this;
}

GraphicsPipelineBuilder&
GraphicsPipelineBuilder::UseDynamicRendering(VkFormat colorFormat, VkFormat depthFormat)
{
    use_render_pass_ = false;
    color_format_ = colorFormat;
    depth_format_ = depthFormat;
    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetInputAssembly(
        const VkPipelineInputAssemblyStateCreateInfo &state)
{
    input_assembly_state_ = state;
    return *this;
}
GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetTessellation(
        bool enable, const VkPipelineTessellationStateCreateInfo &state)
{
    tessellation_enabled_ = enable;
    tessellation_state_ = state;
    return *this;
}

VkPipeline GraphicsPipelineBuilder::Build()
{
    VkGraphicsPipelineCreateInfo pipelineInfo
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = static_cast<uint32_t>(shader_stages_.size()),
        .pStages = shader_stages_.data(),
        .pVertexInputState = &vertex_input_state_,
        .pInputAssemblyState = &input_assembly_state_,
        .pViewportState = &viewport_state_,
        .pRasterizationState = &rasterizer_state_,
        .pMultisampleState = &multisample_state_,
        .pDepthStencilState = &depth_stencil_state_,
        .pColorBlendState = &color_blend_state_,
        .layout = pipeline_layout_,
    };

    VkPipelineRenderingCreateInfo renderingInfo{};
    if (use_render_pass_)
    {
        pipelineInfo.renderPass = render_pass_;
        pipelineInfo.subpass = subpass_;
    }
    else
    {
        renderingInfo =
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &color_format_,
            .depthAttachmentFormat = depth_format_,
        };
        pipelineInfo.pNext = &renderingInfo;
        pipelineInfo.renderPass = VK_NULL_HANDLE;
        pipelineInfo.subpass = 0;
    }

    if (tessellation_enabled_)
    {
        pipelineInfo.pTessellationState = &tessellation_state_;
    };

    auto device = VulkanContext::Get().GetVkDevice();
    VkPipeline pipeline = VK_NULL_HANDLE;
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
    {
        return VK_NULL_HANDLE;
    }
    return pipeline;
}
