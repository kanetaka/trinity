#include "triangle_app.h"
#include "core/vulkan_context.h"
#include "core/swapchain.h"
#include "core/image_barrier.h"
#include <thread>

void TriangleApp::DrawFrame() {
	auto& vulkan_context = VulkanContext::Get();
	auto& swapchain = vulkan_context.GetSwapchain();
	auto device = vulkan_context.GetDevice();

	if (vulkan_context.AcquireNextImage() != VK_SUCCESS) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		return;
	}

	auto* frame_context = vulkan_context.GetCurrentFrameContext();
	auto& command_buffer = frame_context->command_buffer;
	command_buffer->Begin();

	// TODO Implement rendering process here
	VkImageSubresourceRange range {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1,
	};
	command_buffer->TransitionImageLayout(
		swapchain->GetCurrentImage(),
		range,
		ImageLayoutTransition::FromUndefinedToColorAttachment()
		);

	command_buffer->End();

	vulkan_context.SubmitPresent();
}