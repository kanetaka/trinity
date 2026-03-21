#pragma once
#include <vulkan/vulkan.h>
#include <filesystem>

namespace tr {

VkShaderModule LoadShaderModule(VkDevice device, const std::filesystem::path& shaderSpvPath);

} // namespace tr
