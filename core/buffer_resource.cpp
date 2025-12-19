#include "core/buffer_resource.h"


// ------------------------ //
// ---- Public Methods ---- //
// ------------------------ //

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
        buffer_ = VK_NULL_HANDLE;
    }
    if (memory_ != VK_NULL_HANDLE) {
        vkFreeMemory(device, memory_, nullptr);
        memory_ = VK_NULL_HANDLE;
    }
    size_ = 0;
}


// --------------------------- //
// ---- Protected Methods ---- //
// --------------------------- //

template<typename T>
bool BufferResource<T>::CreateBuffer(const VkBufferCreateInfo& buffer_create_info, VkMemoryPropertyFlags memory_props) {
    VulkanContext& context = VulkanContext::Get();
    VkDevice device = context.GetDevice();

    auto result = vkCreateBuffer(device, &buffer_create_info, nullptr, &buffer_);
    if (result != VK_SUCCESS) {
        return false;
    }

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(device, buffer_, &memory_requirements);

    VkMemoryAllocateInfo allocate_info {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = context.FindMemoryType(memory_requirements, memory_props),
    };

	result = vkAllocateMemory(device, &allocate_info, nullptr, &memory_);
    if (result != VK_SUCCESS) {
        return false;
    }

	vkBindBufferMemory(device, buffer_, memory_, 0);
	size_ = buffer_create_info.size;
	memory_props_ = memory_props;

    return true;
}
