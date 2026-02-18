#include "core/vertex_buffer.h"

// ---------------------------------- //
// ---- Constructor/Destructor ---- //
// ---------------------------------- //


// ------------------------------- //
// ---- Public Member Methods ---- //
// ------------------------------- //

bool VertexBuffer::Initialize(VkDeviceSize size, VkMemoryPropertyFlags memory_props) {
    auto& context = VulkanContext::Get();
    VkBufferCreateInfo buffer_info {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = size,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
    };

    SetAccessFlags(VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
    return CreateBuffer(buffer_info, memory_props);
}

void* VertexBuffer::Map() {
    if (!(memory_props_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
        return nullptr;
    }

    void* mapped = nullptr;
    vkMapMemory(VulkanContext::Get().GetDevice(), memory_, 0, size_, 0, &mapped);
    return mapped;
}

void VertexBuffer::Unmap() {
    if (!(memory_props_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
        return;
    }

    vkUnmapMemory(VulkanContext::Get().GetDevice(), memory_);
}