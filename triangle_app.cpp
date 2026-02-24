#include "core/image_barrier.h"
#include "core/swapchain.h"
#include "core/vulkan_context.h"
#include "triangle_app.h"
#include <thread>

void TriangleApp::DrawFrame() {
  auto &vulkan_context = VulkanContext::Get();
  auto &swapchain = vulkan_context.GetSwapchain();
  auto device = vulkan_context.GetDevice();

  if (vulkan_context.AcquireNextImage() != VK_SUCCESS) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return;
  }

  auto *frame_context = vulkan_context.GetCurrentFrameContext();
  auto &command_buffer = frame_context->command_buffer;
  command_buffer->Begin();

  // TODO Implement rendering process here
  VkImageSubresourceRange range{
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .baseMipLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1,
  };
  command_buffer->TransitionImageLayout(
      swapchain->GetCurrentImage(), range,
      ImageLayoutTransition::FromUndefinedToColorAttachment());

  VkClearValue clear_color = {0.0f, 0.0f, 1.0f, 1.0f};
  VkRenderingAttachmentInfo color_attachment{
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .pNext = nullptr,
      .imageView = swapchain->GetCurrentImageView(),
      .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .resolveMode = VK_RESOLVE_MODE_NONE,
      .resolveImageView = VK_NULL_HANDLE,
      .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue = clear_color,
  };
  VkRenderingInfo rendering_info{
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .pNext = nullptr,
      .flags = 0,
      .renderArea =
          {
              .offset = {0, 0},
              .extent = swapchain->GetImageExtent(),
          },
      .layerCount = 1,
      .viewMask = 0,
      .colorAttachmentCount = 1,
      .pColorAttachments = &color_attachment,
      .pDepthAttachment = nullptr,
      .pStencilAttachment = nullptr,
  };
  vkCmdBeginRendering(command_buffer->Get(), &rendering_info);

  // Rendering commands would go here

  command_buffer->TransitionImageLayout(
      swapchain->GetCurrentImage(), range,
      ImageLayoutTransition::FromColorToPresentSrc());
  command_buffer->End();
  vulkan_context.SubmitPresent();
}