#include "render/renderer.h"
#include "render/vulkan_context.h"
#include "render/swapchain.h"
#include "io/shader_loader.h"
#include "core/asset_path.h"
#include "render/pipeline/graphics_pipeline_builder.h"
#include "render/resources/buffer_resource.h"
#include "app/application.h"
#include "core/registry.h"
#include "core/components.h"
#include "render/components/splat_data_component.h"
#include <stdexcept>
#include <algorithm>

namespace tri
{
    Renderer::Renderer(Application* app)
        : app_(app), screen_width_(0), screen_height_(0) {}

    Renderer::~Renderer() {}

    bool Renderer::Initialize(float screen_width, float screen_height)
    {
        screen_width_ = screen_width;
        screen_height_ = screen_height;

        // Create uniform buffer for camera data
        uniform_buffer_ = UniformBuffer::Create(256); // Assuming sizeof(CameraUBO)

        // Create storage buffer for entity transforms (batch transfer)
        // Assuming max 1024 entities for now
        transform_buffer_ = StorageBuffer::Create(sizeof(glm::mat4) * 1024, StorageBuffer::AccessMode::CpuAccessible);

        CreateDescriptorSetLayout();
        CreateDescriptorPool();
        InitializeGraphicsPipeline();

        return true;
    }

    void Renderer::Shutdown()
    {
        auto device = VulkanContext::Get().GetVkDevice();
        vkDeviceWaitIdle(device);

        if (uniform_buffer_)
        {
            uniform_buffer_->Cleanup();
            uniform_buffer_.reset();
        }

        if (pipeline_)
        {
            vkDestroyPipeline(device, pipeline_, nullptr);
            pipeline_ = VK_NULL_HANDLE;
        }
        if (pipeline_layout_)
        {
            vkDestroyPipelineLayout(device, pipeline_layout_, nullptr);
            pipeline_layout_ = VK_NULL_HANDLE;
        }
        if (descriptor_set_layout_)
        {
            vkDestroyDescriptorSetLayout(device, descriptor_set_layout_, nullptr);
            descriptor_set_layout_ = VK_NULL_HANDLE;
        }
        if (descriptor_pool_)
        {
            vkDestroyDescriptorPool(device, descriptor_pool_, nullptr);
            descriptor_pool_ = VK_NULL_HANDLE;
        }
    }

    void Renderer::Draw(Entity root)
    {
        if (root == NullEntity) return;
        auto& vulkan_ctx = VulkanContext::Get();

        if (vulkan_ctx.AcquireNextImage() != VK_SUCCESS)
        {
            return;
        }

        auto* frame_ctx = vulkan_ctx.GetCurrentFrameContext();
        auto& command_buffer = frame_ctx->commandBuffer;
        command_buffer->Begin();

        auto& swapchain = vulkan_ctx.GetSwapchain();
        auto image_view = swapchain->GetCurrentView();
        auto extent = swapchain->GetExtent();

        VkImageSubresourceRange range{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

        command_buffer->TransitionLayout(
            swapchain->GetCurrentImage(), range,
            ImageLayoutTransition::FromUndefinedToColorAttachment());

        VkRenderingAttachmentInfo color_attachment{};
        color_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        color_attachment.imageView = image_view;
        color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        VkClearValue clear_color = { {{0.1f, 0.1f, 0.1f, 1.0f}} };
        color_attachment.clearValue = clear_color;

        VkRenderingInfo rendering_info{};
        rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        rendering_info.renderArea.offset = { 0, 0 };
        rendering_info.renderArea.extent = extent;
        rendering_info.layerCount = 1;
        rendering_info.colorAttachmentCount = 1;
        rendering_info.pColorAttachments = &color_attachment;

        vkCmdBeginRendering(command_buffer->Get(), &rendering_info);

        vkCmdBindPipeline(command_buffer->Get(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);

        DrawEntity(root, app_->GetRegistry(), command_buffer);

        vkCmdEndRendering(command_buffer->Get());

        command_buffer->TransitionLayout(swapchain->GetCurrentImage(),
            range, ImageLayoutTransition::FromColorToPresent());

        command_buffer->End();

        vulkan_ctx.SubmitPresent();
    }

    void Renderer::DrawEntity(Entity entity, Registry& registry, std::shared_ptr<CommandBuffer>& command_buffer)
    {
        if (auto* splat = registry.GetComponent<SplatDataComponent>(entity))
        {
            if (splat->descriptor_set != VK_NULL_HANDLE)
            {
                vkCmdBindDescriptorSets(command_buffer->Get(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, 0, 1, &splat->descriptor_set, 0, nullptr);

                uint32_t transform_index = registry.GetPoolIndex<TransformComponent>(entity);
                vkCmdPushConstants(command_buffer->Get(), pipeline_layout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint32_t), &transform_index);

                // Access original vertex count/index through the splat buffer size or keep it in component
                // For now, assume common 4 vertices per splat as before. 
                // We need to know the number of splats. Let's assume splat_buffer size / sizeof(GpuSplat).
                // But actually we have index_buffer.
                if (splat->index_buffer)
                {
                    uint32_t num_splats = static_cast<uint32_t>(splat->index_buffer->GetBufferSize() / sizeof(uint32_t));
                    vkCmdDraw(command_buffer->Get(), 4, num_splats, 0, 0);
                }
            }
        }

        if (auto* hierarchy = registry.GetComponent<HierarchyComponent>(entity))
        {
            for (auto child_id : hierarchy->children)
            {
                DrawEntity(child_id, registry, command_buffer);
            }
        }
    }

    VkDescriptorSet Renderer::AllocateDescriptorSet()
    {
        auto device = VulkanContext::Get().GetVkDevice();
        VkDescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = descriptor_pool_;
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &descriptor_set_layout_;

        VkDescriptorSet set;
        if (vkAllocateDescriptorSets(device, &alloc_info, &set) != VK_SUCCESS)
        {
            return VK_NULL_HANDLE;
        }
        return set;
    }

    void Renderer::UpdateSplatDescriptorSet(VkDescriptorSet set, const std::shared_ptr<StorageBuffer>& splat_buffer, const std::shared_ptr<StorageBuffer>& index_buffer)
    {
        auto device = VulkanContext::Get().GetVkDevice();

        VkDescriptorBufferInfo ubo_info{};
        ubo_info.buffer = uniform_buffer_->GetVkBuffer();
        ubo_info.offset = 0;
        ubo_info.range = VK_WHOLE_SIZE;

        VkWriteDescriptorSet ubo_write{};
        ubo_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        ubo_write.dstSet = set;
        ubo_write.dstBinding = 0;
        ubo_write.dstArrayElement = 0;
        ubo_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ubo_write.descriptorCount = 1;
        ubo_write.pBufferInfo = &ubo_info;

        VkDescriptorBufferInfo splat_info{};
        splat_info.buffer = splat_buffer->GetVkBuffer();
        splat_info.offset = 0;
        splat_info.range = VK_WHOLE_SIZE;

        VkWriteDescriptorSet splat_write{};
        splat_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        splat_write.dstSet = set;
        splat_write.dstBinding = 1;
        splat_write.dstArrayElement = 0;
        splat_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        splat_write.descriptorCount = 1;
        splat_write.pBufferInfo = &splat_info;

        VkDescriptorBufferInfo idx_info{};
        idx_info.buffer = index_buffer->GetVkBuffer();
        idx_info.offset = 0;
        idx_info.range = VK_WHOLE_SIZE;

        VkWriteDescriptorSet idx_write{};
        idx_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        idx_write.dstSet = set;
        idx_write.dstBinding = 2;
        idx_write.dstArrayElement = 0;
        idx_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        idx_write.descriptorCount = 1;
        idx_write.pBufferInfo = &idx_info;

        VkDescriptorBufferInfo transform_info{};
        transform_info.buffer = transform_buffer_->GetVkBuffer();
        transform_info.offset = 0;
        transform_info.range = VK_WHOLE_SIZE;

        VkWriteDescriptorSet transform_write{};
        transform_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        transform_write.dstSet = set;
        transform_write.dstBinding = 3;
        transform_write.dstArrayElement = 0;
        transform_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        transform_write.descriptorCount = 1;
        transform_write.pBufferInfo = &transform_info;

        std::vector<VkWriteDescriptorSet> writes = { ubo_write, splat_write, idx_write, transform_write };
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
    }

    struct CameraUbo
    {
        glm::mat4 view;
        glm::mat4 proj;
        glm::vec2 viewport;
        glm::vec2 padding;
    };

    void Renderer::UpdateUniformBuffer()
    {
        CameraUbo ubo{};
        ubo.view = view_;
        ubo.proj = projection_;
        ubo.viewport = glm::vec2(screen_width_, screen_height_);

        void* data = uniform_buffer_->Map();
        memcpy(data, &ubo, sizeof(ubo));
        uniform_buffer_->Unmap();
    }

    void Renderer::UpdateTransformBuffer(Registry& registry)
    {
        auto transforms = registry.View<TransformComponent>();
        if (transforms.empty()) return;

        std::vector<glm::mat4> matrices;
        matrices.reserve(transforms.size());
        for (const auto& t : transforms)
        {
            matrices.push_back(t.world_transform);
        }

        void* data = transform_buffer_->Map();
        memcpy(data, matrices.data(), matrices.size() * sizeof(glm::mat4));
        transform_buffer_->Unmap();
    }

    bool Renderer::CreateDescriptorSetLayout()
    {
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

        VkDescriptorSetLayoutBinding transform_binding{};
        transform_binding.binding = 3;
        transform_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        transform_binding.descriptorCount = 1;
        transform_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        std::vector<VkDescriptorSetLayoutBinding> bindings = {
                ubo_binding, splat_binding, idx_binding, transform_binding };
        VkDescriptorSetLayoutCreateInfo layout_info{};
        layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
        layout_info.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(
            device, &layout_info, nullptr, &descriptor_set_layout_) != VK_SUCCESS)
        {
            return false;
        }
        return true;
    }

    bool Renderer::CreateDescriptorPool()
    {
        auto device = VulkanContext::Get().GetVkDevice();

        std::vector<VkDescriptorPoolSize> pool_sizes =
        {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 20 } };

        VkDescriptorPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
        pool_info.pPoolSizes = pool_sizes.data();
        pool_info.maxSets = 10;

        if (vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptor_pool_) != VK_SUCCESS)
        {
            return false;
        }
        return true;
    }

    bool Renderer::InitializeGraphicsPipeline()
    {
        auto& vulkan_ctx = VulkanContext::Get();
        auto device = vulkan_ctx.GetVkDevice();
        auto extent = vulkan_ctx.GetSwapchain()->GetExtent();

        // Load Shaders
        auto vert_module =
            LoadShaderModule(VulkanContext::Get().GetVkDevice(), GetAssetRootPath() / "shaders" / "splat" / "splat.vert.spv");
        auto frag_module =
            LoadShaderModule(VulkanContext::Get().GetVkDevice(), GetAssetRootPath() / "shaders" / "splat" / "splat.frag.spv");

        VkPushConstantRange pushRange{};
        pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushRange.offset = 0;
        pushRange.size = sizeof(uint32_t); // matrixIndex

        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = 1;
        pipeline_layout_info.pSetLayouts = &descriptor_set_layout_;
        pipeline_layout_info.pushConstantRangeCount = 1;
        pipeline_layout_info.pPushConstantRanges = &pushRange;

        if (vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipeline_layout_) != VK_SUCCESS)
        {
            return false;
        }

        GraphicsPipelineBuilder builder;
        pipeline_ = builder.AddShaderStage(VK_SHADER_STAGE_VERTEX_BIT, vert_module, "main")
            .AddShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, frag_module, "main")
            .SetVertexInput(nullptr, 0, nullptr, 0)
            .SetViewport(extent)
            .SetPipelineLayout(pipeline_layout_)
            .UseDynamicRendering(vulkan_ctx.GetSwapchain()->GetFormat().format, VK_FORMAT_UNDEFINED)
            .EnableAlphaBlend()
            .Build();

        vkDestroyShaderModule(device, vert_module, nullptr);
        vkDestroyShaderModule(device, frag_module, nullptr);

        return true;
    }

} // namespace tri
