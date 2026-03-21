#pragma once
#include <string>
#include <memory>
#include <vulkan/vulkan.h>

namespace tr {
class StorageBuffer;

struct SplatDataComponent
{
    std::string ply_file;
    std::shared_ptr<StorageBuffer> splat_buffer;
    std::shared_ptr<StorageBuffer> index_buffer;
    VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
};

} // namespace tr
