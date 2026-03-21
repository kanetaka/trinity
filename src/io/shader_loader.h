#pragma once
#include <vulkan/vulkan.h>
#include <filesystem>

namespace loader
{
    VkShaderModule LoadShaderModule(VkDevice device, const std::filesystem::path& shaderSpvPath);
};
