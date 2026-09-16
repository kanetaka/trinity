#include "graphics/command_buffer.h"

using namespace tri;


CommandBuffer::CommandBuffer(VkDevice device, VkCommandPool command_pool, VkCommandBuffer command_buffer)
{
    device_ = device;
    command_pool_ = command_pool;
    command_buffer_ = command_buffer;
}

CommandBuffer::~CommandBuffer()
{
    if (command_buffer_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE && command_pool_ != VK_NULL_HANDLE)
    {
        vkFreeCommandBuffers(device_, command_pool_, 1, &command_buffer_);
        command_buffer_ = VK_NULL_HANDLE;
    }
}

void CommandBuffer::Begin(VkCommandBufferUsageFlags usage_flag)
{
    VkCommandBufferBeginInfo begin_info
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .flags = usage_flag
    };
    vkBeginCommandBuffer(command_buffer_, &begin_info);
}

void CommandBuffer::End() { vkEndCommandBuffer(command_buffer_); }

void CommandBuffer::Reset() { vkResetCommandBuffer(command_buffer_, 0); }

void CommandBuffer::TransitionLayout(VkImage image,
    const VkImageSubresourceRange& range,
    const ImageLayoutTransition& transition)
{
    VkImageMemoryBarrier2 image_barrier
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = transition.srcStage,
        .srcAccessMask = transition.srcAccessMask,
        .dstStageMask = transition.dstStage,
        .dstAccessMask = transition.dstAccessMask,
        .oldLayout = transition.oldLayout,
        .newLayout = transition.newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = range
    };

    VkDependencyInfo dependency_info
    {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &image_barrier
    };

    vkCmdPipelineBarrier2(command_buffer_, &dependency_info);
}
