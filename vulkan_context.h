#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <functional>
#include <stdint.h>
#include <string>
#include <cstring>

#include "core/command_buffer.h"

class Swapchain;
class CommandBuffer;
class ISurfaceProvider;

class VulkanContext {
public:
    static constexpr uint32_t MaxInflightFrames = 2;
    static VulkanContext& Get(); // TODO GetInstance

    void Initialize(const char* app_name, ISurfaceProvider* surface_provider);
private:
    VulkanContext() = default;
    ~VulkanContext() = default;

    ISurfaceProvider* surface_provider_{};
};