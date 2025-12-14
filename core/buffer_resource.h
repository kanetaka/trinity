#pragma once
#include "core/vulkan_context.h"
#include "core/gpu_resource_base.h"

class IBufferResource {
public:
    virtual bool IsHostAccessible() const = 0;
    virtual VkBuffer GetBuffer() const = 0;
    virtual VkDeviceSize GetBufferSize() const = 0;

    virtual void SetAccessFlags(const VkAccessFlags access_flags) = 0;
    virtual VkAccessFlags GetAccessFlags() const = 0;

    virtual void* Map() = 0;
    virtual void Unmap() = 0;

    virtual VkDescriptorBufferInfo GetDescriptorBufferInfo() const = 0;
};

template<typename T>
class BufferResource : public GpuResourceBase<T>, public IBufferResource {
public:
    BufferResource(const BufferResource&) = delete;
    BufferResource& operator=(const BufferResource&) = delete;
    virtual ~BufferResource() { Cleanrup(); }

protected:
	BufferResource() = default;

public:
    bool IsHostAccessible() const override { return (memory_props_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0; }
    VkAccessFlags GetAccessFlags() const override { return access_flags_; }
	void SetAccessFlags(const VkAccessFlags access_flags) override { access_flags_ = access_flags; }
    VkBuffer GetBuffer() const override { return buffer_; }
    VkDeviceSize GetBufferSize() const override { return size_; }
    VkDescriptorBufferInfo GetDescriptorBufferInfo() const override;

	virtual void Cleanrup();

protected:
    bool CreateBuffer(const VkBufferCreateInfo& buffer_create_info, VkMemoryPropertyFlags memory_props);

protected:
    VkBuffer buffer_{};
    VkDeviceMemory memory_{};
    VkDeviceSize size_{};
    VkMemoryPropertyFlags memory_props_{};
    VkAccessFlags access_flags_ = VK_ACCESS_NONE;
};
