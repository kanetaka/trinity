#pragma once
#include "graphics/graphics_context.h"
#include "graphics/image_barrier.h"

namespace tri
{
    class CommandBuffer
    {
    public:
        CommandBuffer(VkDevice device, VkCommandPool command_pool, VkCommandBuffer command_buffer);
        virtual ~CommandBuffer();

        void Begin(VkCommandBufferUsageFlags usage_flag = 0);
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
        VkDevice device_ = VK_NULL_HANDLE;
        VkCommandPool command_pool_ = VK_NULL_HANDLE;
        VkCommandBuffer command_buffer_ = VK_NULL_HANDLE;
    };
} // namespace tri
