#pragma once
#include "core/vulkan_context.h"
#include "core/gpu_resource_base.h"

class IBufferResource
{
public:
    virtual bool IsHostAccessible() const = 0;
    virtual VkBuffer GetVkBuffer() const = 0;
    virtual VkDeviceSize GetBufferSize() const = 0;

    virtual void SetAccessFlags(const VkAccessFlags flags) = 0;
    virtual VkAccessFlags GetAccessFlags() const = 0;

    virtual void* Map() = 0;
    virtual void Unmap() = 0;

    virtual VkDescriptorBufferInfo GetDescriptorInfo() const = 0;
};

template<typename T>
class BufferResource : public GpuResourceBase<T>, public IBufferResource
{
public:
    BufferResource(const BufferResource&) = delete;
    BufferResource& operator=(const BufferResource&) = delete;

    virtual ~BufferResource() { Cleanup(); }
    virtual void Cleanup();

    bool IsHostAccessible() const override                  { return (mem_props_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0; }
    VkAccessFlags GetAccessFlags() const override           { return access_flags_; }
    void SetAccessFlags(const VkAccessFlags flags) override { access_flags_ = flags; }

    VkBuffer GetVkBuffer() const override { return buffer_; }
    VkDeviceSize GetBufferSize() const override { return size_; }

    VkDescriptorBufferInfo GetDescriptorInfo() const override;
protected:
    BufferResource() = default;

    bool CreateBuffer(const VkBufferCreateInfo& createInfo, VkMemoryPropertyFlags memProps);
    VkBuffer buffer_{};
    VkDeviceMemory memory_{};
    VkDeviceSize size_{};
    VkMemoryPropertyFlags mem_props_{};
    VkAccessFlags access_flags_ = VK_ACCESS_NONE;
};

class VertexBuffer : public BufferResource<VertexBuffer>
{
    friend class GpuResourceBase<VertexBuffer>;
private:
    VertexBuffer() = default;
public:
    virtual ~VertexBuffer() = default;

    virtual void* Map() override;
    virtual void Unmap() override;

    bool Initialize(VkDeviceSize size, VkMemoryPropertyFlags memProps);

    // Static factory method for creation and initialization
    static std::shared_ptr<VertexBuffer> Create(VkDeviceSize size, VkMemoryPropertyFlags memProps)
    {
        auto buffer = GpuResourceBase::Create();
        if (!buffer->Initialize(size, memProps)) { return nullptr; }
        return buffer;
    }
};

class IndexBuffer : public BufferResource<IndexBuffer>
{
    friend class GpuResourceBase<IndexBuffer>;
private:
    IndexBuffer() = default;
public:
    virtual ~IndexBuffer() = default;

    virtual void* Map() override;
    virtual void Unmap() override;

    bool Initialize(VkDeviceSize size, VkMemoryPropertyFlags memProps);

    // Static factory method for creation and initialization
    static std::shared_ptr<IndexBuffer> Create(VkDeviceSize size, VkMemoryPropertyFlags memProps)
    {
        auto buffer = GpuResourceBase::Create();
        if (!buffer->Initialize(size, memProps)) { return nullptr; }
        return buffer;
    }
};

class UniformBuffer : public BufferResource<UniformBuffer>
{
    friend class GpuResourceBase<UniformBuffer>;
public:
    UniformBuffer() = default;
    virtual ~UniformBuffer() = default;

    virtual void* Map() override;
    virtual void Unmap() override;

    bool Initialize(VkDeviceSize size);

    // Static factory method for creation and initialization
    static std::shared_ptr<UniformBuffer> Create(VkDeviceSize size)
    {
        auto buffer = GpuResourceBase::Create();
        if (!buffer->Initialize(size)) { return nullptr; }
        return buffer;
    }
};

class DynamicUniformBuffer : public BufferResource<DynamicUniformBuffer>
{
    friend class GpuResourceBase<DynamicUniformBuffer>;
public:
    DynamicUniformBuffer() = default;
    virtual ~DynamicUniformBuffer() = default;

    virtual void* Map() override;
    virtual void Unmap() override;

    bool Initialize(VkDeviceSize size);

    // Static factory method for creation and initialization
    static std::shared_ptr<DynamicUniformBuffer> Create(VkDeviceSize size)
    {
        auto buffer = GpuResourceBase::Create();
        if (!buffer->Initialize(size)) { return nullptr; }
        return buffer;
    }

    VkDescriptorBufferInfo GetDescriptorInfo() const override;
    uint32_t GetCurrentOffset() const;
private:
    uint32_t block_size_ = 0;
};

// Staging Buffer
class StagingBuffer : public BufferResource<StagingBuffer>
{
    friend class GpuResourceBase<StagingBuffer>;
private:
    StagingBuffer() = default;

public:
    virtual ~StagingBuffer() = default;

    virtual void* Map() override;
    virtual void Unmap() override;

    bool Initialize(VkDeviceSize size);

    // Static factory method for creation and initialization
    static std::shared_ptr<StagingBuffer> Create(VkDeviceSize size)
    {
        auto buffer = GpuResourceBase::Create();
        if (!buffer->Initialize(size)) { return nullptr; }
        return buffer;
    }
};

class StorageBuffer : public BufferResource<StorageBuffer>
{
    friend class GpuResourceBase<StorageBuffer>;
private:
    StorageBuffer() = default;
public:
    virtual ~StorageBuffer() = default;

    virtual void* Map() override;
    virtual void Unmap() override;

    enum class AccessMode
    {
        GPUOnlyAccess,
        CPUAccessible,
    };

    bool Initialize(VkDeviceSize size, AccessMode mode);

    template<typename T>
    T* MapTyped() { return reinterpret_cast<T*>(Map()); }

    // Static factory method for creation and initialization
    static std::shared_ptr<StorageBuffer> Create(VkDeviceSize size, AccessMode mode)
    {
        auto buffer = GpuResourceBase::Create();
        if (!buffer->Initialize(size, mode)) { return nullptr; }
        return buffer;
    }
};
