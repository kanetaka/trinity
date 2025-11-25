#include "vulkan_context.h"

VulkanContext& VulkanContext::Get() {
    static VulkanContext instance;
    return instance;
}

void VulkanContext::Initialize(const char* app_name, ISurfaceProvider* surface_provider) {
    surface_provider_ = surface_provider;
}