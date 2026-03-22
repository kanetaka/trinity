#pragma once
#include "render/vulkan_context.h"
#include "render/image_barrier.h"

namespace tri
{
    class CommandBuffer
    {
    public:
        CommandBuffer(VkCommandBuffer commandBuffer);
        virtual ~CommandBuffer();

        void Begin(VkCommandBufferUsageFlags usageFlag = 0);
        void End();
        void Reset();

        VkCommandBuffer Get() const { return command_buffer_; }

        operator VkCommandBuffer() { return command_buffer_; }
        operator VkCommandBuffer() const { return command_buffer_; }

        void TransitionLayout(VkImage image, const VkImageSubresourceRange& range, const ImageLayoutTransition& transition);


        template<typename T>
        void TransitionLayout(std::shared_ptr<T> image, const ImageLayoutTransition& transition)
        {
            TransitionLayout(image->GetVkImage(), image->GetSubresourceRange(), transition);
            image->SetAccessFlag(transition.dstAccessMask);
            image->SetLayout(transition.newLayout);
        }
    private:
        VkCommandBuffer command_buffer_{};
    };
} // namespace tri
