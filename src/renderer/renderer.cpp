#include "renderer/renderer.h"
#include "graphics/graphics_context.h"
#include "graphics/swapchain.h"
#include "scene/io/shader_loader.h"
#include "core/asset_path.h"
#include "graphics/pipeline/graphics_pipeline_builder.h"
#include "graphics/resource/buffer_resource.h"
#include "scene/object.h"
#include "scene/scene.h"
#include "scene/component/i_renderable.h"
#include <stdexcept>
#include <algorithm>

using namespace tri;

Renderer::Renderer(GraphicsContext& context)
    : context_(context)
    , screen_width_(0)
    , screen_height_(0)
{
}

Renderer::~Renderer()
{
}

bool Renderer::Initialize(float screen_width, float screen_height)
{
    if (screen_width <= 0.0f || screen_height <= 0.0f)
    {
        auto extent = context_.GetSwapchain()->GetExtent();
        screen_width_ = static_cast<float>(extent.width);
        screen_height_ = static_cast<float>(extent.height);
    }
    else
    {
        screen_width_ = screen_width;
        screen_height_ = screen_height;
    }

    // Create uniform buffer for camera data
    uniform_buffer_ = UniformBuffer::Create(context_, 256); // Assuming sizeof(CameraUBO)

    // Create storage buffer for entity transforms (batch transfer)
    // Assuming max 1024 entities for now
    transform_buffer_ = StorageBuffer::Create(context_, sizeof(glm::mat4) * 1024, StorageBuffer::AccessMode::CpuAccessible);

    CreateDescriptorSetLayout();
    CreateDescriptorPool();
    InitializeGraphicsPipeline();

    return true;
}

void Renderer::WaitIdle()
{
    context_.WaitIdle();
}

void Renderer::Shutdown()
{
    context_.WaitIdle();
    auto device = context_.GetVkDevice();

    if (uniform_buffer_)
    {
        uniform_buffer_->Cleanup();
        uniform_buffer_.reset();
    }

    if (transform_buffer_)
    {
        transform_buffer_->Cleanup();
        transform_buffer_.reset();
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

void Renderer::Draw(Object& root, std::function<void(std::shared_ptr<CommandBuffer>&)> post_render)
{
    auto& graphics_ctx = context_;

    if (graphics_ctx.AcquireNextImage() != VK_SUCCESS)
    {
        return;
    }

    auto* frame_ctx = graphics_ctx.GetCurrentFrameContext();
    auto& command_buffer = frame_ctx->commandBuffer;
    command_buffer->Begin();

    auto& swapchain = graphics_ctx.GetSwapchain();
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

    std::vector<RenderItem> render_items;
    CollectRenderItems(root, render_items);

    std::stable_sort(render_items.begin(), render_items.end(),
        [](const RenderItem& a, const RenderItem& b)
        {
            return a.render_order < b.render_order;
        });

    for (const auto& item : render_items)
    {
        item.renderable->Render(*command_buffer, pipeline_layout_, item.transform_index);
    }

    if (post_render)
    {
        post_render(command_buffer);
    }

    vkCmdEndRendering(command_buffer->Get());

    command_buffer->TransitionLayout(swapchain->GetCurrentImage(),
        range, ImageLayoutTransition::FromColorToPresent());

    command_buffer->End();

    graphics_ctx.SubmitPresent();
}

void Renderer::CollectRenderItems(Object& object, std::vector<RenderItem>& out_items)
{
    object.ForEachComponent<IRenderable>([&](IRenderable& renderable)
        {
            int order = 0;
            if (auto* comp = dynamic_cast<IComponent*>(&renderable))
            {
                order = comp->GetRenderOrder();
                auto it = type_orders_.find(std::type_index(typeid(*comp)));
                if (it != type_orders_.end())
                {
                    order += it->second;
                }
            }
            out_items.push_back(RenderItem{ &renderable, object.GetTransformIndex(), order });
        });

    for (auto& child : object.GetChildren())
    {
        CollectRenderItems(*child, out_items);
    }
}

VkDescriptorSet Renderer::AllocateDescriptorSet()
{
    auto device = context_.GetVkDevice();
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

void Renderer::RegisterComponent(IComponent& component, int render_order)
{
    if (render_order != 0)
    {
        component.SetRenderOrder(render_order);
    }
    else
    {
        auto it = type_orders_.find(std::type_index(typeid(component)));
        if (it != type_orders_.end())
        {
            component.SetRenderOrder(it->second);
        }
    }

    VkDescriptorSet set = AllocateDescriptorSet();
    component.SetupResources(set, uniform_buffer_->GetVkBuffer(), transform_buffer_->GetVkBuffer());
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

void Renderer::UpdateTransformBuffer(const Scene& scene)
{
    const auto& objects = scene.GetOrderedObjects();
    if (objects.empty()) return;

    std::vector<glm::mat4> matrices;
    matrices.reserve(objects.size());
    for (const auto* object : objects)
    {
        glm::dmat4 rel = object->GetWorldTransform();
        rel[3][0] -= camera_pos_.x;
        rel[3][1] -= camera_pos_.y;
        rel[3][2] -= camera_pos_.z;
        matrices.push_back(glm::mat4(rel));
    }

    void* data = transform_buffer_->Map();
    memcpy(data, matrices.data(), matrices.size() * sizeof(glm::mat4));
    transform_buffer_->Unmap();
}

bool Renderer::CreateDescriptorSetLayout()
{
    auto device = context_.GetVkDevice();

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
    auto device = context_.GetVkDevice();

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
    auto& graphics_ctx = context_;
    auto device = graphics_ctx.GetVkDevice();
    auto extent = graphics_ctx.GetSwapchain()->GetExtent();

    // Load Shaders
    auto vert_module =
        LoadShaderModule(device, GetAssetRootPath() / "shader" / "splat" / "splat.vert.spv");
    auto frag_module =
        LoadShaderModule(device, GetAssetRootPath() / "shader" / "splat" / "splat.frag.spv");

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
        .UseDynamicRendering(graphics_ctx.GetSwapchain()->GetFormat().format, VK_FORMAT_UNDEFINED)
        .EnableAlphaBlend()
        .Build(device);

    vkDestroyShaderModule(device, vert_module, nullptr);
    vkDestroyShaderModule(device, frag_module, nullptr);

    return true;
}
