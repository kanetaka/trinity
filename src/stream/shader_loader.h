#pragma once
#include <vulkan/vulkan.h>
#include <filesystem>

namespace tri
{
    VkShaderModule LoadShaderModule(VkDevice device, const std::filesystem::path& shaderSpvPath);
} // namespace tri
