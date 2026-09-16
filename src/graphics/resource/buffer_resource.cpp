#include "graphics/resource/buffer_resource.h"

using namespace tri;

template<typename T>
void BufferResource<T>::Cleanup()
{
    if (context_)
    {
        VkDevice device = context_->GetVkDevice();

        if (buffer_ != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(device, buffer_, nullptr);
            buffer_ = VK_NULL_HANDLE;
        }
        if (memory_ != VK_NULL_HANDLE)
        {
            vkFreeMemory(device, memory_, nullptr);
            memory_ = VK_NULL_HANDLE;
        }
        context_ = nullptr;
    }
    size_ = 0;
}

template<typename T>
VkDescriptorBufferInfo BufferResource<T>::GetDescriptorInfo() const
{
    return VkDescriptorBufferInfo
    {
        .buffer = buffer_,
        .offset = 0,
        .range = size_
    };
}

template<typename T>
bool BufferResource<T>::CreateBuffer(GraphicsContext& context, const VkBufferCreateInfo& create_info, VkMemoryPropertyFlags mem_prop_flags)
{
    context_ = &context;
    VkDevice device = context_->GetVkDevice();

    auto result = vkCreateBuffer(device, &create_info, nullptr, &buffer_);
    if (result != VK_SUCCESS)
    {
        return false;
    }

    // Get memory requirements
    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(device, buffer_, &mem_requirements);

    VkMemoryAllocateInfo alloc_info
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = context_->FindMemoryType(mem_requirements, mem_prop_flags),
    };

    result = vkAllocateMemory(device, &alloc_info, nullptr, &memory_);
    if (result != VK_SUCCESS)
    {
        return false;
    }

    vkBindBufferMemory(device, buffer_, memory_, 0);
    size_ = create_info.size;
    mem_props_ = mem_prop_flags;

    return true;
}

bool VertexBuffer::Initialize(GraphicsContext& context, VkDeviceSize size, VkMemoryPropertyFlags mem_prop_flags)
{
    VkBufferCreateInfo buffer_info
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    SetAccessFlags(VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
    return CreateBuffer(context, buffer_info, mem_prop_flags);
}

void* VertexBuffer::Map()
{
    if (!context_ || !(mem_props_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) return nullptr;

    void* mapped = nullptr;
    vkMapMemory(context_->GetVkDevice(), memory_, 0, size_, 0, &mapped);
    return mapped;
}

void VertexBuffer::Unmap()
{
    if (!context_ || !(mem_props_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) return;

    vkUnmapMemory(context_->GetVkDevice(), memory_);
}

bool IndexBuffer::Initialize(GraphicsContext& context, VkDeviceSize size, VkMemoryPropertyFlags mem_prop_flags)
{
    VkBufferCreateInfo buffer_info
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    SetAccessFlags(VK_ACCESS_INDEX_READ_BIT);
    return CreateBuffer(context, buffer_info, mem_prop_flags);
}

void* IndexBuffer::Map()
{
    if (!context_ || !(mem_props_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) return nullptr;

    void* mapped = nullptr;
    vkMapMemory(context_->GetVkDevice(), memory_, 0, size_, 0, &mapped);
    return mapped;
}

void IndexBuffer::Unmap()
{
    if (!context_ || !(mem_props_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) return;

    vkUnmapMemory(context_->GetVkDevice(), memory_);
}

bool UniformBuffer::Initialize(GraphicsContext& context, VkDeviceSize size)
{
    VkBufferCreateInfo buffer_info
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    VkMemoryPropertyFlags mem_prop_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    SetAccessFlags(VK_ACCESS_SHADER_READ_BIT);
    return CreateBuffer(context, buffer_info, mem_prop_flags);
}

void* UniformBuffer::Map()
{
    if (!context_) return nullptr;
    void* mapped = nullptr;
    vkMapMemory(context_->GetVkDevice(), memory_, 0, size_, 0, &mapped);
    return mapped;
}

void UniformBuffer::Unmap()
{
    if (!context_) return;
    vkUnmapMemory(context_->GetVkDevice(), memory_);
}

bool StagingBuffer::Initialize(GraphicsContext& context, VkDeviceSize size)
{
    VkBufferCreateInfo buffer_info
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    VkMemoryPropertyFlags mem_prop_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    SetAccessFlags(VK_ACCESS_HOST_WRITE_BIT);
    return CreateBuffer(context, buffer_info, mem_prop_flags);
}

void* StagingBuffer::Map()
{
    if (!context_) return nullptr;
    void* mapped = nullptr;
    VkDevice device = context_->GetVkDevice();
    vkMapMemory(device, memory_, 0, size_, 0, &mapped);
    return mapped;
}

void StagingBuffer::Unmap()
{
    if (!context_) return;
    VkDevice device = context_->GetVkDevice();
    vkUnmapMemory(device, memory_);
}

void* DynamicUniformBuffer::Map()
{
    if (!context_) return nullptr;
    VkDevice device = context_->GetVkDevice();
    auto frame_index = context_->GetCurrentFrameIndex();
    auto offset = frame_index * block_size_;

    void* mapped = nullptr;
    vkMapMemory(device, memory_, offset, block_size_, 0, &mapped);
    return mapped;
}

void DynamicUniformBuffer::Unmap()
{
    if (!context_) return;
    VkDevice device = context_->GetVkDevice();
    auto frame_index = context_->GetCurrentFrameIndex();
    auto offset = frame_index * block_size_;

    VkMappedMemoryRange mapped_range
    {
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .memory = memory_,
        .offset = offset,
        .size = block_size_,
    };
    vkFlushMappedMemoryRanges(device, 1, &mapped_range);
    vkUnmapMemory(device, memory_);
}

bool DynamicUniformBuffer::Initialize(GraphicsContext& context, VkDeviceSize size)
{
    auto ubo_offset_alignment = context.MinUniformOffsetAlignment();
    auto non_coherent_atom_size = context.NonCoherentAtomSize();
    auto align_size = std::max(ubo_offset_alignment, non_coherent_atom_size);

    block_size_ = (size + align_size - 1ULL) & ~(align_size - 1ULL);
    VkDeviceSize buffer_size = block_size_ * GraphicsContext::MaxInflightFrames;
    VkBufferCreateInfo buffer_info
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = buffer_size,
        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    VkMemoryPropertyFlags mem_prop_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    SetAccessFlags(VK_ACCESS_SHADER_READ_BIT);
    return CreateBuffer(context, buffer_info, mem_prop_flags);
}

VkDescriptorBufferInfo DynamicUniformBuffer::GetDescriptorInfo() const
{
    return VkDescriptorBufferInfo
    {
        .buffer = buffer_,
        .offset = 0,
        .range = block_size_,
    };
}

uint32_t DynamicUniformBuffer::GetCurrentOffset() const
{
    if (!context_) return 0;
    VkDeviceSize offset = block_size_ * context_->GetCurrentFrameIndex();
    return uint32_t(offset);
}

void* StorageBuffer::Map()
{
    if (!context_ || !(mem_props_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) return nullptr;

    void* mapped = nullptr;
    vkMapMemory(context_->GetVkDevice(), memory_, 0, size_, 0, &mapped);
    return mapped;
}

void StorageBuffer::Unmap()
{
    if (!context_ || !(mem_props_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) return;

    vkUnmapMemory(context_->GetVkDevice(), memory_);
}

bool StorageBuffer::Initialize(GraphicsContext& context, VkDeviceSize size, AccessMode mode)
{
    VkBufferCreateInfo buffer_info
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    VkMemoryPropertyFlags mem_prop_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    if (mode == AccessMode::CpuAccessible)
    {
        mem_prop_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }
    SetAccessFlags(VK_ACCESS_NONE);
    return CreateBuffer(context, buffer_info, mem_prop_flags);
}


// Explicit template instantiation
template class BufferResource<VertexBuffer>;
template class BufferResource<IndexBuffer>;
template class BufferResource<UniformBuffer>;
template class BufferResource<StagingBuffer>;
template class BufferResource<DynamicUniformBuffer>;
template class BufferResource<StorageBuffer>;
