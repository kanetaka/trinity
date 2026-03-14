#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class GraphicsPipelineBuilder {
public:
    GraphicsPipelineBuilder();

    // 各ステージ追加
    GraphicsPipelineBuilder &AddShaderStage(VkShaderStageFlagBits stage,
                                                                                    VkShaderModule module,
                                                                                    const char *entry = "main");

    // 頂点入力レイアウト
    GraphicsPipelineBuilder &
    SetVertexInput(const VkVertexInputBindingDescription *bindings,
                                 uint32_t bindingCount,
                                 const VkVertexInputAttributeDescription *attributes,
                                 uint32_t attributeCount);

    // ビューポートとシザー
    GraphicsPipelineBuilder &SetViewport(VkExtent2D extent);
    GraphicsPipelineBuilder &SetViewport(VkViewport &viewport, VkRect2D scissor);

    // ブレンディング設定
    void SetColorBlendAttachmentState(
            const VkPipelineColorBlendAttachmentState &state);
    GraphicsPipelineBuilder &EnableAlphaBlend();

    // ラスタライズ設定
    void
    SetRasterizationState(const VkPipelineRasterizationStateCreateInfo &state);

    // デプス・ステンシル設定
    void SetDepthStencilState(const VkPipelineDepthStencilStateCreateInfo &state);

    // レイアウト
    GraphicsPipelineBuilder &SetPipelineLayout(VkPipelineLayout layout);

    // VkRenderPassを使用する場合の設定
    GraphicsPipelineBuilder &UseRenderPass(VkRenderPass renderPass,
                                                                                 uint32_t subpass);

    // DynamicRenderingを使用する場合の設定
    GraphicsPipelineBuilder &
    UseDynamicRendering(VkFormat colorFormat,
                                            VkFormat depthFormat = VK_FORMAT_UNDEFINED);

    // パイプライン作成
    VkPipeline Build();

    // 入力アセンブリを変更(テッセレーションなどで使用)
    GraphicsPipelineBuilder &
    SetInputAssembly(const VkPipelineInputAssemblyStateCreateInfo &state);

    // テッセレーション情報の設定
    GraphicsPipelineBuilder &
    SetTessellation(bool enable,
                                    const VkPipelineTessellationStateCreateInfo &state);

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
