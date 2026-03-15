#include "core/graphics/command_buffer.h"

CommandBuffer::CommandBuffer(VkCommandBuffer commandBuffer) {
  command_buffer_ = commandBuffer;
}

CommandBuffer::~CommandBuffer() {
  auto &vulkan_ctx = VulkanContext::Get();
  vkFreeCommandBuffers(vulkan_ctx.GetVkDevice(), vulkan_ctx.GetCommandPool(), 1,
                       &command_buffer_);
  command_buffer_ = VK_NULL_HANDLE;
}

void CommandBuffer::Begin(VkCommandBufferUsageFlags usageFlag) {
  VkCommandBufferBeginInfo beginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .flags = usageFlag

  };
  vkBeginCommandBuffer(command_buffer_, &beginInfo);
}

void CommandBuffer::End() { vkEndCommandBuffer(command_buffer_); }

void CommandBuffer::Reset() { vkResetCommandBuffer(command_buffer_, 0); }

void CommandBuffer::TransitionLayout(VkImage image,
                                     const VkImageSubresourceRange &range,
                                     const ImageLayoutTransition &transition) {
  VkImageMemoryBarrier2 imageBarrier{
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
      .subresourceRange = range};

  VkDependencyInfo dependencyInfo{.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                                  .imageMemoryBarrierCount = 1,
                                  .pImageMemoryBarriers = &imageBarrier};

  vkCmdPipelineBarrier2(command_buffer_, &dependencyInfo);
}
