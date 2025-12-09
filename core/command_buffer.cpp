#include "core/command_buffer.h"

// -------------------------------- //
// ---- Constructor/Destructor ---- //
// -------------------------------- //

CommandBuffer::CommandBuffer(VkCommandBuffer command_buffer)
    : command_buffer_(command_buffer) {
}

CommandBuffer::~CommandBuffer() {
    auto& vulkan_context = VulkanContext::Get();
    vkFreeCommandBuffers(vulkan_context.GetDevice(), vulkan_context.GetCommandPool(), 1, &command_buffer_);
    command_buffer_ = VK_NULL_HANDLE;
}

// ------------------------------- //
// ---- Public Member Methods ---- //
// ------------------------------- //

void CommandBuffer::Begin(VkCommandBufferUsageFlags usage_flags) {
    VkCommandBufferBeginInfo begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = usage_flags,
        .pInheritanceInfo = nullptr,
    };
    vkBeginCommandBuffer(command_buffer_, &begin_info);
}

void CommandBuffer::End() {
    vkEndCommandBuffer(command_buffer_);
}

void CommandBuffer::Reset() {
    vkResetCommandBuffer(command_buffer_, 0);
}

void CommandBuffer::TransitionImageLayout(VkImage image, const VkImageSubresourceRange& range, const ImageLayoutTransition& transition) {
    VkImageMemoryBarrier2 image_barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = transition.src_stage,
        .srcAccessMask = transition.src_access_mask,
        .dstStageMask = transition.dst_stage,
        .dstAccessMask = transition.dst_access_mask,
        .oldLayout = transition.old_layout,
        .newLayout = transition.new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = range,
    };

    VkDependencyInfo dependency_info{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .pNext = nullptr,
        .dependencyFlags = 0,
        .memoryBarrierCount = 0,
        .pMemoryBarriers = nullptr,
        .bufferMemoryBarrierCount = 0,
        .pBufferMemoryBarriers = nullptr,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &image_barrier,
    };

    vkCmdPipelineBarrier2(command_buffer_, &dependency_info);
}

// ------------------------------- //
// ---- Private Member Methods ---- //
// ------------------------------- //
