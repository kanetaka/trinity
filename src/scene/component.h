#pragma once
#include <vulkan/vulkan.h>

namespace tri
{
    // Interface implemented by every component that can be attached to an Object.
    class IComponent
    {
    public:
        virtual ~IComponent() = default;

        int GetRenderOrder() const { return render_order_; }
        void SetRenderOrder(int order) { render_order_ = order; }

        virtual void SetupResources(VkDescriptorSet descriptor_set, VkBuffer ubo, VkBuffer transform_buffer)
        {
            (void)descriptor_set;
            (void)ubo;
            (void)transform_buffer;
        }

    private:
        int render_order_ = 0;
    };
} // namespace tri
