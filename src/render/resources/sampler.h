#pragma once
#include "render/vulkan_context.h"
#include "render/resources/gpu_resource_base.h"

namespace tri
{
    class Sampler : public GpuResourceBase<Sampler>
    {
        friend class GpuResourceBase<Sampler>;
    private:
        Sampler() = default;
    public:
        Sampler(const Sampler&) = delete;
        Sampler& operator=(const Sampler&) = delete;

        virtual ~Sampler() { Cleanup(); }
        void Initialize(
            VkFilter minFilter,
            VkFilter magFilter,
            VkSamplerMipmapMode mipmapMode,
            VkSamplerAddressMode addrModeU,
            VkSamplerAddressMode addrModeV,
            float minLod = 0.0f,
            float maxLod = VK_LOD_CLAMP_NONE);
        void Cleanup();

        VkSampler GetVkSampler() const { return sampler_; }
        operator const VkSampler& () { return sampler_; }

    protected:
        VkSampler sampler_ = VK_NULL_HANDLE;
    };


} // namespace tri
