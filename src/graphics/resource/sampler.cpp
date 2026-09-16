#include "graphics/resource/sampler.h"
#include <stdexcept>

using namespace tri;

void Sampler::Initialize(
    GraphicsContext& context,
    VkFilter minFilter,
    VkFilter magFilter,
    VkSamplerMipmapMode mipmapMode,
    VkSamplerAddressMode addrModeU,
    VkSamplerAddressMode addrModeV,
    float minLod,
    float maxLod)
{
    context_ = &context;
    VkSamplerCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = magFilter,
        .minFilter = minFilter,
        .mipmapMode = mipmapMode,
        .addressModeU = addrModeU,
        .addressModeV = addrModeV,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .maxAnisotropy = 1.0f,
        .minLod = minLod,
        .maxLod = maxLod,
    };

    auto result = vkCreateSampler(context_->GetVkDevice(), &createInfo, nullptr, &sampler_);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to sampler !");
    }
}

void Sampler::Cleanup()
{
    if (context_ && sampler_ != VK_NULL_HANDLE)
    {
        vkDestroySampler(context_->GetVkDevice(), sampler_, nullptr);
        context_ = nullptr;
    }
    sampler_ = VK_NULL_HANDLE;
}
