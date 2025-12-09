#pragma once
#include <vulkan/vulkan.h>

struct ImageLayoutTransition {
	VkImageLayout old_layout;
	VkImageLayout new_layout;
	VkAccessFlags src_access_mask;
	VkAccessFlags dst_access_mask;
	VkPipelineStageFlags src_stage;
	VkPipelineStageFlags dst_stage;

	static ImageLayoutTransition FromUndefinedToColorAttachment();
	static ImageLayoutTransition FromPresentSrcToColorAttachment();
	static ImageLayoutTransition FromColorToPresentSrc();
};