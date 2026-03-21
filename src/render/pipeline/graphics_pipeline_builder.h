#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace tr {




class GraphicsPipelineBuilder
{
public:
    GraphicsPipelineBuilder();

    // Add shader stages
    GraphicsPipelineBuilder& AddShaderStage(VkShaderStageFlagBits stage, VkShaderModule module, const char* entry = "main");

    // Vertex input layout
    GraphicsPipelineBuilder&
        SetVertexInput(const VkVertexInputBindingDescription* bindings,
            uint32_t bindingCount,
            const VkVertexInputAttributeDescription* attributes,
            uint32_t attributeCount);

    // Viewport and scissor
    GraphicsPipelineBuilder& SetViewport(VkExtent2D extent);
    GraphicsPipelineBuilder& SetViewport(VkViewport& viewport, VkRect2D scissor);

    // Blending configuration
    void SetColorBlendAttachmentState(
        const VkPipelineColorBlendAttachmentState& state);
    GraphicsPipelineBuilder& EnableAlphaBlend();

    // Rasterization configuration
    void
        SetRasterizationState(const VkPipelineRasterizationStateCreateInfo& state);

    // Depth and stencil configuration
    void SetDepthStencilState(const VkPipelineDepthStencilStateCreateInfo& state);

    // Pipeline layout
    GraphicsPipelineBuilder& SetPipelineLayout(VkPipelineLayout layout);

    // Configuration for using VkRenderPass
    GraphicsPipelineBuilder& UseRenderPass(VkRenderPass renderPass, uint32_t subpass);

    // Configuration for using dynamic rendering
    GraphicsPipelineBuilder&
        UseDynamicRendering(VkFormat colorFormat, VkFormat depthFormat = VK_FORMAT_UNDEFINED);

    // Build the pipeline
    VkPipeline Build();

    // Change input assembly (e.g., for tessellation)
    GraphicsPipelineBuilder&
        SetInputAssembly(const VkPipelineInputAssemblyStateCreateInfo& state);

    // Tessellation configuration
    GraphicsPipelineBuilder& SetTessellation(bool enable, const VkPipelineTessellationStateCreateInfo& state);

private:
    VkDevice device_;

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages_;

    VkPipelineVertexInputStateCreateInfo vertex_input_state_{};
    std::vector<VkVertexInputBindingDescription> binding_descriptions_;
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions_;

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_{};
    VkPipelineViewportStateCreateInfo viewport_state_{};
    VkViewport viewport_{};
    VkRect2D scissor_{};

    VkPipelineRasterizationStateCreateInfo rasterizer_state_{};
    VkPipelineMultisampleStateCreateInfo multisample_state_{};
    VkPipelineColorBlendAttachmentState color_blend_attachment_{};
    VkPipelineColorBlendStateCreateInfo color_blend_state_{};
    VkPipelineDepthStencilStateCreateInfo depth_stencil_state_{};

    bool tessellation_enabled_ = false;
    VkPipelineTessellationStateCreateInfo tessellation_state_{};

    VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;
    VkFormat color_format_ = VK_FORMAT_UNDEFINED;
    VkFormat depth_format_ = VK_FORMAT_UNDEFINED;

    bool use_render_pass_ = false;
    VkRenderPass render_pass_ = VK_NULL_HANDLE;
    uint32_t subpass_ = 0;
};


} // namespace tr
