#pragma once
#include <cstdint>
#include <vulkan/vulkan.h>

namespace tri
{
    class CommandBuffer;

    // Implemented by components that record their own draw calls.
    class IRenderable
    {
    public:
        virtual ~IRenderable() = default;
        virtual void Render(CommandBuffer& command_buffer, VkPipelineLayout pipeline_layout, uint32_t transform_index) const = 0;
    };
} // namespace tri
