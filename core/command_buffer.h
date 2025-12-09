#pragma once

#include "core/vulkan_context.h"
#include "core/image_barrier.h"

class CommandBuffer {
public:
    CommandBuffer(VkCommandBuffer command_buffer);
    virtual ~CommandBuffer();

public:
	void Begin(VkCommandBufferUsageFlags usage_flags = 0);
    void End();
    void Reset();

    VkCommandBuffer Get() const { return command_buffer_; }
    void TransitionImageLayout(VkImage image, const VkImageSubresourceRange& range, const ImageLayoutTransition& transition);

	operator VkCommandBuffer() { return command_buffer_; }
	operator VkCommandBuffer() const { return command_buffer_; }

private:
    VkCommandBuffer command_buffer_{};

};
