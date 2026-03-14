#pragma once
#include "core/vulkan_context.h"
#include <filesystem>

namespace loader
{
    VkShaderModule LoadShaderModule(const std::filesystem::path& shaderSpvPath);
};
