#include "gs_app.h"
#include "core/asset_path.h"
#include "core/graphics_pipeline_builder.h"
#include "core/ply_loader.h"
#include "core/shader_loader.h"
#include "core/swapchain.h"
#include <algorithm>
#include <execution>
#include <iostream>

struct CameraUBO {
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec2 viewport;
    glm::vec2 padding;
};

GsApp::GsApp(const std::string &plyFile)
        : ply_file_(plyFile), camera_(glm::vec3(0.0f, 0.0f, 5.0f),
                                                                     glm::vec3(0.0f, -1.0f, 0.0f), -90.0f, 0.0f) {
}

GsApp::~GsApp() {}

void GsApp::OnInitialize() {
    LoadSplats();
    CreateBuffers();
    CreateDescriptorSetLayout();
    CreateDescriptorPool();
    CreateDescriptorSets();
    InitializeGraphicsPipeline();
}

void GsApp::LoadSplats() {
    std::vector<gs::FullSplat> splats;
    if (!gs::PlyLoader::LoadPly(ply_file_, splats)) {
        std::cerr << "Failed to load PLY. Using empty splat list." << std::endl;
        return;
    }

    gpu_splats_.reserve(splats.size());
    splat_indices_.reserve(splats.size());

    // Compute scene bounds
    glm::vec3 min_pt(std::numeric_limits<float>::max());
    glm::vec3 max_pt(std::numeric_limits<float>::lowest());

    for (uint32_t i = 0; i < splats.size(); ++i) {
        const auto &s = splats[i];
        gs::GPUSplat gpu_splat;
        gpu_splat.position_opacity = glm::vec4(s.position, s.opacity);
        gpu_splat.rot_scale_0 = glm::vec4(s.rot.x, s.rot.y, s.rot.z, s.scale.x);
        gpu_splat.rot_w_scale_yz = glm::vec4(s.rot.w, s.scale.y, s.scale.z, 0.0f);
        gpu_splat.sh_dc = glm::vec4(s.sh_dc[0], s.sh_dc[1], s.sh_dc[2], 0.0f);

        gpu_splats_.push_back(gpu_splat);
        splat_indices_.push_back({i, 0.0f});

        min_pt = glm::min(min_pt, s.position);
        max_pt = glm::max(max_pt, s.position);
    }

    // Auto-center camera
    glm::vec3 center = (min_pt + max_pt) * 0.5f;
    float radius = glm::length(max_pt - center);
    camera_.Position = center + glm::vec3(0.0f, 0.0f, radius * 1.5f);
}

void GsApp::CreateBuffers() {
    auto &vulkan_ctx = VulkanContext::Get();

    // 1. Uniform Buffer
    uniform_buffer_ = UniformBuffer::Create(sizeof(CameraUBO));

    // 2. Splat Storage Buffer (Read-only on GPU)
    if (!gpu_splats_.empty()) {
        VkDeviceSize splat_buffer_size = gpu_splats_.size() * sizeof(gs::GPUSplat);
        splat_buffer_ = StorageBuffer::Create(
                splat_buffer_size, StorageBuffer::AccessMode::CPUAccessible);

        void *data = splat_buffer_->Map();
        memcpy(data, gpu_splats_.data(), splat_buffer_size);
        splat_buffer_->Unmap();

        // 3. Index Buffer (Storage Buffer)
        VkDeviceSize index_buffer_size = splat_indices_.size() * sizeof(uint32_t);
        index_buffer_ = StorageBuffer::Create(
                index_buffer_size, StorageBuffer::AccessMode::CPUAccessible);
    }
}

void GsApp::CreateDescriptorSetLayout() {
    auto device = VulkanContext::Get().GetVkDevice();

    VkDescriptorSetLayoutBinding ubo_binding{};
    ubo_binding.binding = 0;
    ubo_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_binding.descriptorCount = 1;
    ubo_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding splat_binding{};
    splat_binding.binding = 1;
    splat_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    splat_binding.descriptorCount = 1;
    splat_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding idx_binding{};
    idx_binding.binding = 2;
    idx_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    idx_binding.descriptorCount = 1;
    idx_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
            ubo_binding, splat_binding, idx_binding};
    VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
    layout_info.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layout_info, nullptr,
                                                                    &descriptor_set_layout_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void GsApp::CreateDescriptorPool() {
    auto device = VulkanContext::Get().GetVkDevice();

    std::vector<VkDescriptorPoolSize> pool_sizes = {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2}};

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    pool_info.pPoolSizes = pool_sizes.data();
    pool_info.maxSets = 1;

    if (vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptor_pool_) !=
            VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void GsApp::CreateDescriptorSets() {
    auto device = VulkanContext::Get().GetVkDevice();

    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = descriptor_pool_;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &descriptor_set_layout_;

    if (vkAllocateDescriptorSets(device, &alloc_info, &descriptor_set_) !=
            VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    VkDescriptorBufferInfo ubo_info{};
    ubo_info.buffer = uniform_buffer_->GetVkBuffer();
    ubo_info.offset = 0;
    ubo_info.range = sizeof(CameraUBO);

    VkWriteDescriptorSet ubo_write{};
    ubo_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    ubo_write.dstSet = descriptor_set_;
    ubo_write.dstBinding = 0;
    ubo_write.dstArrayElement = 0;
    ubo_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_write.descriptorCount = 1;
    ubo_write.pBufferInfo = &ubo_info;

    std::vector<VkWriteDescriptorSet> descriptor_writes = {ubo_write};

    VkDescriptorBufferInfo splat_info{};
    VkWriteDescriptorSet splat_write{};
    VkDescriptorBufferInfo idx_info{};
    VkWriteDescriptorSet idx_write{};

    if (splat_buffer_ && index_buffer_) {
        splat_info.buffer = splat_buffer_->GetVkBuffer();
        splat_info.offset = 0;
        splat_info.range = VK_WHOLE_SIZE;

        splat_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        splat_write.dstSet = descriptor_set_;
        splat_write.dstBinding = 1;
        splat_write.dstArrayElement = 0;
        splat_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        splat_write.descriptorCount = 1;
        splat_write.pBufferInfo = &splat_info;

        idx_info.buffer = index_buffer_->GetVkBuffer();
        idx_info.offset = 0;
        idx_info.range = VK_WHOLE_SIZE;

        idx_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        idx_write.dstSet = descriptor_set_;
        idx_write.dstBinding = 2;
        idx_write.dstArrayElement = 0;
        idx_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        idx_write.descriptorCount = 1;
        idx_write.pBufferInfo = &idx_info;

        descriptor_writes.push_back(splat_write);
        descriptor_writes.push_back(idx_write);
    }

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptor_writes.size()),
                                                 descriptor_writes.data(), 0, nullptr);
}

void GsApp::InitializeGraphicsPipeline() {
    auto &vulkan_ctx = VulkanContext::Get();
    auto device = vulkan_ctx.GetVkDevice();
    auto extent = vulkan_ctx.GetSwapchain()->GetExtent();

    width_ = (float)extent.width;
    height_ = (float)extent.height;

    // Load Shaders
    auto vert_module =
            loader::LoadShaderModule(GetAssetRootPath() / "splat.vert.spv");
    auto frag_module =
            loader::LoadShaderModule(GetAssetRootPath() / "splat.frag.spv");

    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &descriptor_set_layout_;

    if (vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr,
                                                         &pipeline_layout_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    GraphicsPipelineBuilder builder;
    pipeline_ =
            builder.AddShaderStage(VK_SHADER_STAGE_VERTEX_BIT, vert_module, "main")
                    .AddShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, frag_module, "main")
                    // No vertex input state, we use gl_VertexIndex and gl_InstanceIndex
                    .SetVertexInput(nullptr, 0, nullptr, 0)
                    .SetViewport(extent)
                    .SetPipelineLayout(pipeline_layout_)
                    .UseDynamicRendering(vulkan_ctx.GetSwapchain()->GetFormat().format,
                                                             VK_FORMAT_UNDEFINED)
                    .EnableAlphaBlend()
                    .Build();

    vkDestroyShaderModule(device, vert_module, nullptr);
    vkDestroyShaderModule(device, frag_module, nullptr);
}

void GsApp::SortSplats() {
    if (splat_indices_.empty())
        return;

    glm::mat4 view = camera_.GetViewMatrix();
    // Compute depth for all splats
    for (size_t i = 0; i < splat_indices_.size(); ++i) {
        uint32_t initial_idx = splat_indices_[i].index;
        const auto &pos = gpu_splats_[initial_idx].position_opacity;
        glm::vec4 view_pos = view * glm::vec4(pos.x, pos.y, pos.z, 1.0f);
        splat_indices_[i].depth = view_pos.z;
    }

    // Sort far to near (Vulkan view Z is negative into screen, so larger Z is
    // closer, meaning we sort ascending Z? Wait, GLM lookAt uses right-handed, so
    // forward is -Z. Far means more negative Z. We want to render back-to-front.
    // Back is more negative Z. So we sort ascending (smallest Z first).
    std::sort(std::execution::par_unseq, splat_indices_.begin(),
                        splat_indices_.end(),
                        [](const gs::SplatSortEntry &a, const gs::SplatSortEntry &b) {
                            return a.depth < b.depth;
                        });

    // Upload indices to SSBO
    if (index_buffer_) {
        void *data = index_buffer_->Map();
        // We only map the uint32_t indices. We can extract them or pack them
        // Actually, splat_indices_ is an array of structs (uint32_t, float).
        // Our SSBO expects an array of uints.
        uint32_t *mapped_uints = reinterpret_cast<uint32_t *>(data);
        for (size_t i = 0; i < splat_indices_.size(); ++i) {
            mapped_uints[i] = splat_indices_[i].index;
        }
        index_buffer_->Unmap();
    }
}

void GsApp::UpdateUniformBuffer() {
    CameraUBO ubo{};
    ubo.view = camera_.GetViewMatrix();
    ubo.proj = camera_.GetProjectionMatrix(width_ / height_);
    ubo.viewport = glm::vec2(width_, height_);

    void *data = uniform_buffer_->Map();
    memcpy(data, &ubo, sizeof(ubo));
    uniform_buffer_->Unmap();
}

void GsApp::OnDrawFrame() {
    if (splat_indices_.empty())
        return;

    SortSplats();
    UpdateUniformBuffer();

    auto &vulkan_ctx = VulkanContext::Get();

    if (vulkan_ctx.AcquireNextImage() != VK_SUCCESS) {
        return; // Swapchain recreated or failed
    }

    auto *frame_ctx = vulkan_ctx.GetCurrentFrameContext();
    auto &command_buffer = frame_ctx->commandBuffer;
    command_buffer->Begin();

    auto &swapchain = vulkan_ctx.GetSwapchain();
    auto image_view = swapchain->GetCurrentView();
    auto extent = swapchain->GetExtent();

    VkImageSubresourceRange range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    command_buffer->TransitionLayout(
            swapchain->GetCurrentImage(), range,
            ImageLayoutTransition::FromUndefinedToColorAttachment());

    VkRenderingAttachmentInfo color_attachment{};
    color_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    VkClearValue clear_color = {{{0.1f, 0.1f, 0.1f, 1.0f}}};
    color_attachment.clearValue = clear_color;

    VkRenderingInfo rendering_info{};
    rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    rendering_info.renderArea.offset = {0, 0};
    rendering_info.renderArea.extent = extent;
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;

    vkCmdBeginRendering(*command_buffer, &rendering_info);

    vkCmdBindPipeline(*command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        pipeline_);
    vkCmdBindDescriptorSets(*command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                    pipeline_layout_, 0, 1, &descriptor_set_, 0, nullptr);

    // Draw 4 vertices (quad) for each splat
    vkCmdDraw(*command_buffer, 4, static_cast<uint32_t>(splat_indices_.size()), 0,
                        0);

    vkCmdEndRendering(*command_buffer);

    command_buffer->TransitionLayout(swapchain->GetCurrentImage(), range,
                                                                    ImageLayoutTransition::FromColorToPresent());

    command_buffer->End();

    vulkan_ctx.SubmitPresent();
}

void GsApp::OnCleanup() {
    auto device = VulkanContext::Get().GetVkDevice();
    vkDeviceWaitIdle(device);

    if (pipeline_) {
        vkDestroyPipeline(device, pipeline_, nullptr);
        pipeline_ = VK_NULL_HANDLE;
    }
    if (pipeline_layout_) {
        vkDestroyPipelineLayout(device, pipeline_layout_, nullptr);
        pipeline_layout_ = VK_NULL_HANDLE;
    }
    if (descriptor_set_layout_) {
        vkDestroyDescriptorSetLayout(device, descriptor_set_layout_, nullptr);
        descriptor_set_layout_ = VK_NULL_HANDLE;
    }
    if (descriptor_pool_) {
        vkDestroyDescriptorPool(device, descriptor_pool_, nullptr);
        descriptor_pool_ = VK_NULL_HANDLE;
    }

    if (splat_buffer_) {
        splat_buffer_->Cleanup();
        splat_buffer_.reset();
    }
    if (index_buffer_) {
        index_buffer_->Cleanup();
        index_buffer_.reset();
    }
    if (uniform_buffer_) {
        uniform_buffer_->Cleanup();
        uniform_buffer_.reset();
    }
}

void GsApp::ProcessInput(const Uint8 *state, float deltaTime) {
    camera_.ProcessKeyboard(state, deltaTime);
}

void GsApp::ProcessMouseMotion(float xrel, float yrel) {
    camera_.ProcessMouseMovement(xrel, -yrel); // Invert y
}

#if defined(__ANDROID__)
void GsApp::OnSurfaceChanged() {
    auto &vulkan_ctx = VulkanContext::Get();
    vulkan_ctx.RecreateSwapchain();
    auto extent = vulkan_ctx.GetSwapchainExtent();
    width_ = (float)extent.width;
    height_ = (float)extent.height;
}
#endif
