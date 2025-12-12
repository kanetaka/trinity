#include "core/buffer_resource.h"

template<typename T>
VkDescriptorBufferInfo BufferResource<T>::GetDescriptorBufferInfo() const {
	return VkDescriptorBufferInfo{
		.buffer = buffer_,
		.offset = 0,
		.range = size_,
	};
}

template<typename T>
void BufferResource<T>::Cleanrup() {
	VulkanContext& context = VulkanContext::Get();
	VkDevice device = context.GetDevice();

	if (buffer_ != VK_NULL_HANDLE) {
		vkDestroyBuffer(device, buffer_, nullptr);
		buffer = VK_NULL_HANDLE;
	}
	if (memory!!= VK_NULL_HANDLE) {
		vkFreeMemory(device, memory_, nullptr);
		memory_ = VK_NULL_HANDLE;
	}
	size_ = 0;
}

