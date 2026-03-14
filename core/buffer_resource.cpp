#include "core/buffer_resource.h"

template<typename T>
void BufferResource<T>::Cleanup()
{
    VulkanContext& context = VulkanContext::Get();
    VkDevice device = context.GetVkDevice();

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

template<typename T>
VkDescriptorBufferInfo BufferResource<T>::GetDescriptorInfo() const
{
    return VkDescriptorBufferInfo{
        .buffer = buffer_,
        .offset = 0,
        .range = size_
    };
}

template<typename T>
bool BufferResource<T>::CreateBuffer(const VkBufferCreateInfo& createInfo, VkMemoryPropertyFlags memProps)
{
    VulkanContext& context = VulkanContext::Get();
    VkDevice device = context.GetVkDevice();

    auto result = vkCreateBuffer(device, &createInfo, nullptr, &buffer_);
    if(result != VK_SUCCESS) {
        return false;
    }

    // Get memory requirements
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer_, &memRequirements);

    VkMemoryAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = context.FindMemoryType(memRequirements, memProps),
    };

    result = vkAllocateMemory(device, &allocInfo, nullptr, &memory_);
    if(result != VK_SUCCESS) {
        return false;
    }

    vkBindBufferMemory(device, buffer_, memory_, 0);
    size_ = createInfo.size;
    mem_props_ = memProps;

    return true;
}

bool VertexBuffer::Initialize(VkDeviceSize size, VkMemoryPropertyFlags memProps)
{
    auto& context = VulkanContext::Get();
    VkBufferCreateInfo bufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    SetAccessFlags(VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
    return CreateBuffer(bufferInfo, memProps);
}

void* VertexBuffer::Map()
{
    if (!(mem_props_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) return nullptr;

    void* mapped = nullptr;
    vkMapMemory(VulkanContext::Get().GetVkDevice(), memory_, 0, size_, 0, &mapped);
    return mapped;
}

void VertexBuffer::Unmap()
{
    if (!(mem_props_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) return;

    vkUnmapMemory(VulkanContext::Get().GetVkDevice(), memory_);
}

bool IndexBuffer::Initialize(VkDeviceSize size, VkMemoryPropertyFlags memProps)
{
    VkBufferCreateInfo bufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    SetAccessFlags(VK_ACCESS_INDEX_READ_BIT);
    return CreateBuffer(bufferInfo, memProps);
}

void* IndexBuffer::Map()
{
    if (!(mem_props_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) return nullptr;

    void* mapped = nullptr;
    vkMapMemory(VulkanContext::Get().GetVkDevice(), memory_, 0, size_, 0, &mapped);
    return mapped;
}

void IndexBuffer::Unmap()
{
    if (!(mem_props_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) return;

    vkUnmapMemory(VulkanContext::Get().GetVkDevice(), memory_);
}

bool UniformBuffer::Initialize(VkDeviceSize size)
{
    VkBufferCreateInfo bufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    SetAccessFlags(VK_ACCESS_SHADER_READ_BIT);
    return CreateBuffer(bufferInfo, memProps);
}

void* UniformBuffer::Map()
{
    void* mapped = nullptr;
    vkMapMemory(VulkanContext::Get().GetVkDevice(), memory_, 0, size_, 0, &mapped);
    return mapped;
}

void UniformBuffer::Unmap()
{
    vkUnmapMemory(VulkanContext::Get().GetVkDevice(), memory_);
}

bool StagingBuffer::Initialize(VkDeviceSize size)
{
    VkBufferCreateInfo bufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    SetAccessFlags(VK_ACCESS_HOST_WRITE_BIT);
    return CreateBuffer(bufferInfo, memProps);
}

void* StagingBuffer::Map()
{
    void* mapped = nullptr;
    VkDevice device = VulkanContext::Get().GetVkDevice();
    vkMapMemory(device, memory_, 0, size_, 0, &mapped);
    return mapped;
}

void StagingBuffer::Unmap()
{
    VkDevice device = VulkanContext::Get().GetVkDevice();
    vkUnmapMemory(device, memory_);
}

void* DynamicUniformBuffer::Map()
{
    auto& vulkanCtx = VulkanContext::Get();
    VkDevice device = vulkanCtx.GetVkDevice();
    auto frameIndex = vulkanCtx.GetCurrentFrameIndex();
    auto offset = frameIndex * block_size_;

    void* mapped = nullptr;
    vkMapMemory(device, memory_, offset, block_size_, 0, &mapped);
    return mapped;
}

void DynamicUniformBuffer::Unmap()
{
    auto& vulkanCtx = VulkanContext::Get();
    VkDevice device = vulkanCtx.GetVkDevice();
    auto frameIndex = vulkanCtx.GetCurrentFrameIndex();
    auto offset = frameIndex * block_size_;

    VkMappedMemoryRange mappedRange{
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .memory = memory_,
        .offset = offset,
        .size = block_size_,
    };
    vkFlushMappedMemoryRanges(device, 1, &mappedRange);
    vkUnmapMemory(device, memory_);
}

bool DynamicUniformBuffer::Initialize(VkDeviceSize size)
{
    auto& vulkanCtx = VulkanContext::Get();
    auto uboOffsetAlignment = vulkanCtx.MinUniformOffsetAlignment();
    auto nonCoherentAtomSize = vulkanCtx.NonCoherentAtomSize();

    auto alignSize = std::max(uboOffsetAlignment, nonCoherentAtomSize);

    block_size_ = (size + alignSize - 1ULL) & ~(alignSize - 1ULL);
    VkDeviceSize bufferSize = block_size_ * vulkanCtx.MaxInflightFrames;
    VkBufferCreateInfo bufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    SetAccessFlags(VK_ACCESS_SHADER_READ_BIT);
    return CreateBuffer(bufferInfo, memProps);
}

VkDescriptorBufferInfo DynamicUniformBuffer::GetDescriptorInfo() const
{
    return VkDescriptorBufferInfo{
        .buffer = buffer_,
        .offset = 0,
        .range = block_size_,
    };
}

uint32_t DynamicUniformBuffer::GetCurrentOffset() const
{
    auto& vulkanCtx = VulkanContext::Get();
    VkDeviceSize offset = block_size_ * vulkanCtx.GetCurrentFrameIndex();
    return uint32_t(offset);
}

void* StorageBuffer::Map()
{
    if (!(mem_props_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) return nullptr;

    void* mapped = nullptr;
    vkMapMemory(VulkanContext::Get().GetVkDevice(), memory_, 0, size_, 0, &mapped);
    return mapped;
}

void StorageBuffer::Unmap()
{
    if (!(mem_props_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) return;

    vkUnmapMemory(VulkanContext::Get().GetVkDevice(), memory_);
}

bool StorageBuffer::Initialize(VkDeviceSize size, AccessMode mode)
{
    VkBufferCreateInfo bufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    if (mode == AccessMode::CPUAccessible) {
        memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }
    SetAccessFlags(VK_ACCESS_NONE);
    return CreateBuffer(bufferInfo, memProps);
}


// Explicit template instantiation
template class BufferResource<VertexBuffer>;
template class BufferResource<IndexBuffer>;
template class BufferResource<UniformBuffer>;
template class BufferResource<StagingBuffer>;
template class BufferResource<DynamicUniformBuffer>;
template class BufferResource<StorageBuffer>;

