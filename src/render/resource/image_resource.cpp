#include "render/resource/image_resource.h"
#include <stdexcept>

using namespace tri;


bool DepthBuffer::Initialize(VkExtent2D extent, VkFormat depthFormat)
{
    auto& vulkanCtx = VulkanContext::Get();
    auto device = vulkanCtx.GetVkDevice();

    format_ = depthFormat;
    extent_ = extent;
    mip_levels_ = 1;

    VkImageCreateInfo createInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format_,
        .extent = { extent.width, extent.height, 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    if (vkCreateImage(device, &createInfo, nullptr, &image_) != VK_SUCCESS)
    {
        return false;
    }

    // Get memory requirements
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image_, &memRequirements);

    VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkMemoryAllocateInfo allocInfo
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = vulkanCtx.FindMemoryType(memRequirements, memProps)
    };
    if (vkAllocateMemory(device, &allocInfo, nullptr, &memory_) != VK_SUCCESS)
    {
        return false;
    }

    if (vkBindImageMemory(device, image_, memory_, 0) != VK_SUCCESS)
    {
        return false;
    }

    subresource_range_ = {
        .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
        .baseMipLevel = 0, .levelCount = createInfo.mipLevels,
        .baseArrayLayer = 0, .layerCount = 1,
    };

    // Create image view
    VkImageViewCreateInfo viewCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image_,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = createInfo.format,
        .subresourceRange = subresource_range_,
    };
    if (vkCreateImageView(device, &viewCreateInfo, nullptr, &image_view_) != VK_SUCCESS)
    {
        return false;
    }

    return true;
}

void DepthBuffer::Cleanup()
{
    auto& vulkanCtx = VulkanContext::Get();
    auto device = vulkanCtx.GetVkDevice();

    if (image_view_ != VK_NULL_HANDLE)
    {
        vkDestroyImageView(device, image_view_, nullptr);
    }
    if (image_ != VK_NULL_HANDLE)
    {
        vkDestroyImage(device, image_, nullptr);
    }
    if (memory_ != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, memory_, nullptr);
    }
    image_ = VK_NULL_HANDLE;
    image_view_ = VK_NULL_HANDLE;
    memory_ = VK_NULL_HANDLE;
}

bool Texture2D::Initialize(VkExtent2D extent, VkFormat format, uint32_t mipLevels)
{
    auto& vulkanCtx = VulkanContext::Get();
    auto device = vulkanCtx.GetVkDevice();

    format_ = format;
    extent_ = extent;
    mip_levels_ = mipLevels;

    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    VkImageCreateInfo createInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format_,
        .extent = { extent.width, extent.height, 1 },
        .mipLevels = mipLevels,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    if (vkCreateImage(device, &createInfo, nullptr, &image_) != VK_SUCCESS)
    {
        return false;
    }

    // Get memory requirements
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image_, &memRequirements);
    VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkMemoryAllocateInfo allocInfo
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = vulkanCtx.FindMemoryType(memRequirements, memProps)
    };
    if (vkAllocateMemory(device, &allocInfo, nullptr, &memory_) != VK_SUCCESS)
    {
        return false;
    }

    if (vkBindImageMemory(device, image_, memory_, 0) != VK_SUCCESS)
    {
        return false;
    }

    subresource_range_ =
    {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0, .levelCount = mipLevels,
        .baseArrayLayer = 0, .layerCount = 1,
    };

    // Create image view
    VkImageViewCreateInfo viewCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image_,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format_,
        .subresourceRange = subresource_range_,
    };
    if (vkCreateImageView(device, &viewCreateInfo, nullptr, &image_view_) != VK_SUCCESS)
    {
        return false;
    }

    return true;
}

void Texture2D::Cleanup()
{
    auto& vulkanCtx = VulkanContext::Get();
    auto device = vulkanCtx.GetVkDevice();
    if (image_view_ != VK_NULL_HANDLE)
    {
        vkDestroyImageView(device, image_view_, nullptr);
    }
    if (image_ != VK_NULL_HANDLE)
    {
        vkDestroyImage(device, image_, nullptr);
    }
    if (memory_ != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, memory_, nullptr);
    }
    image_ = VK_NULL_HANDLE;
    image_view_ = VK_NULL_HANDLE;
    memory_ = VK_NULL_HANDLE;
}

VkDescriptorImageInfo Texture2D::GetDescriptorInfo(VkSampler sampler) const
{
    return VkDescriptorImageInfo
    {
        .sampler = sampler,
        .imageView = image_view_,
        .imageLayout = layout_,
    };
}

bool StorageImage2D::Initialize(VkExtent2D extent, VkFormat format, uint32_t mipLevels)
{
    auto& vulkanCtx = VulkanContext::Get();
    auto device = vulkanCtx.GetVkDevice();

    format_ = format;
    extent_ = extent;
    mip_levels_ = mipLevels;

    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;

    VkImageCreateInfo createInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format_,
        .extent = { extent.width, extent.height, 1 },
        .mipLevels = mipLevels,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    if (vkCreateImage(device, &createInfo, nullptr, &image_) != VK_SUCCESS)
    {
        return false;
    }

    // Get memory requirements
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image_, &memRequirements);
    VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkMemoryAllocateInfo allocInfo
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = vulkanCtx.FindMemoryType(memRequirements, memProps)
    };
    if (vkAllocateMemory(device, &allocInfo, nullptr, &memory_) != VK_SUCCESS)
    {
        return false;
    }

    if (vkBindImageMemory(device, image_, memory_, 0) != VK_SUCCESS)
    {
        return false;
    }

    // Create image view
    subresource_range_ =
    {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0, .levelCount = mipLevels,
        .baseArrayLayer = 0, .layerCount = 1,
    };
    VkImageViewCreateInfo viewCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image_,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format_,
        .subresourceRange = subresource_range_,
    };
    if (vkCreateImageView(device, &viewCreateInfo, nullptr, &image_view_) != VK_SUCCESS)
    {
        return false;
    }
    return true;
}
void StorageImage2D::Cleanup()
{
    auto& vulkanCtx = VulkanContext::Get();
    auto device = vulkanCtx.GetVkDevice();
    if (image_view_ != VK_NULL_HANDLE)
    {
        vkDestroyImageView(device, image_view_, nullptr);
    }
    if (image_ != VK_NULL_HANDLE)
    {
        vkDestroyImage(device, image_, nullptr);
    }
    if (memory_ != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, memory_, nullptr);
    }
    image_ = VK_NULL_HANDLE;
    image_view_ = VK_NULL_HANDLE;
    memory_ = VK_NULL_HANDLE;
}

VkDescriptorImageInfo StorageImage2D::GetTextureReadDescriptorInfo(VkSampler sampler) const
{
    return VkDescriptorImageInfo
    {
        .sampler = sampler,
        .imageView = image_view_,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
}
VkDescriptorImageInfo StorageImage2D::GetStorageReadWriteDescriptorInfo(VkSampler sampler) const
{
    return VkDescriptorImageInfo
    {
        .sampler = sampler,
        .imageView = image_view_,
        .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
    };
}


// Explicit template instantiation
template class ImageResource<DepthBuffer>;
template class ImageResource<Texture2D>;
template class ImageResource<StorageImage2D>;
