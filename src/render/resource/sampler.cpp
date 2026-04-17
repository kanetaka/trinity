#include "render/resource/sampler.h"
#include <stdexcept>

using namespace tri;

void Sampler::Initialize(
    VkFilter minFilter,
    VkFilter magFilter,
    VkSamplerMipmapMode mipmapMode,
    VkSamplerAddressMode addrModeU,
    VkSamplerAddressMode addrModeV,
    float minLod,
    float maxLod)
{
    auto& vulkan_ctx = VulkanContext::Get();
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

    auto result = vkCreateSampler(vulkan_ctx.GetVkDevice(), &createInfo, nullptr, &sampler_);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to sampler !");
    }
}

void Sampler::Cleanup()
{
    auto& vulkan_ctx = VulkanContext::Get();
    if (sampler_ != VK_NULL_HANDLE)
    {
        vkDestroySampler(vulkan_ctx.GetVkDevice(), sampler_, nullptr);
    }
    sampler_ = VK_NULL_HANDLE;
}
