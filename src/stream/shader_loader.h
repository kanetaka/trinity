#pragma once
#include <vulkan/vulkan.h>
#include <filesystem>

namespace trinity::stream {




namespace loader
{
    VkShaderModule LoadShaderModule(VkDevice device, const std::filesystem::path& shaderSpvPath);
};


} // namespace trinity::stream
