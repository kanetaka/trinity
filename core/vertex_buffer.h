#pragma once
#include "core/buffer_resource.h"

class VertexBuffer : public BufferResource<VertexBuffer> {
	friend class GpuResourceBase<VertexBuffer>;
private:
	VertexBuffer() = default;
public:
    virtual ~VertexBuffer() = default;

public:
    static std::shared_ptr<VertexBuffer> Create(VkDeviceSize size, VkMemoryPropertyFlags memory_props) {
        auto buffer = GpuResourceBase::Create();
        if (!buffer->Initialize(size, memory_props)) {
            return nullptr;
        }
        return buffer;
	}

    bool Initialize(VkDeviceSize size, VkMemoryPropertyFlags memory_props);

    virtual void* Map() override;
    virtual void Unmap() override;
};
