#include "io/shader_loader.h"
#include <vector>
#include <fstream>

namespace loader
{
    VkShaderModule LoadShaderModule(VkDevice device, const std::filesystem::path& shaderSpvPath)
    {
        // Read file in binary mode
        std::ifstream file(shaderSpvPath, std::ios::ate | std::ios::binary);
        if (!file.is_open())
        {
            throw std::runtime_error("failed to open shader file: " + shaderSpvPath.string());
        }

        size_t file_size = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(file_size);
        file.seekg(0).read(buffer.data(), file_size);
        file.close();

        // Set binary data to VkShaderModuleCreateInfo
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = buffer.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

        
        VkShaderModule shader_module{};
        auto result = vkCreateShaderModule(device, &createInfo, nullptr, &shader_module);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create shader module from: " + shaderSpvPath.string());
        }
        return shader_module;
    }
}
