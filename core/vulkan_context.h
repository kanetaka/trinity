#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <functional>
#include <stdint.h>
#include <string>
#include <cstring>

#include "command_buffer.h"

class Swapchain;
class CommandBuffer;
class ISurfaceProvider;

class VulkanContext {
public:
    static constexpr uint32_t MaxInflightFrames = 2;
    static VulkanContext& Get(); // TODO GetContext

public:
    void Initialize(const char* app_name, ISurfaceProvider* surface_provider);
    void Cleanup();
    void RecreateSwapchain();

    VkInstance GetInstance() const { return instance_; }
    VkDevice GetDevice() const { return device_; }
    VkPhysicalDevice GetPhysicalDevice() const { return physical_device_; }
    VkDescriptorPool GetDescriptorPool() const { return descriptor_pool_; }

    VkQueue GetGraphicsQueue() const { return graphics_queue_; }
    uint32_t GetGraphicsQueueFamilyIndex() const { return graphics_queue_family_index_; }
    uint32_t GetPresentQueueFamilyIndex() const { return present_queue_family_index_; }

    VkCommandPool GetCommandPool() const { return command_pool_; }
    VkSurfaceKHR GetSurface() const { return surface_; }

    std::shared_ptr<CommandBuffer> CreateCommandBuffer();

    VkDescriptorSet AllocateDescriptorSet(VkDescriptorSetLayout layout);
    void FreeDescriptorSet(VkDescriptorSet descriptor_set);

    struct FrameContext {
        std::shared_ptr<CommandBuffer> command_buffer;
        VkFence inflight_fence = VK_NULL_HANDLE;
    };

    uint32_t GetCurrentFrameIndex() const { return current_frame_index_; }

    VkResult AcquireNextImage();

    void SubmitPresent();
    void SubmitAndWait(std::shared_ptr<CommandBuffer> command_buffer);
    FrameContext* GetCurrentFrameContext();

    std::unique_ptr<Swapchain>& GetSwapchain() { return swapchain_; }
    uint32_t FindMemoryType(const VkMemoryRequirements& requirements, VkMemoryPropertyFlags propeties) const;
    std::function<void(std::vector<const char*>&)> GetWindowSystemExtensions;

    void SetDebugObjectName(void* object_handle, VkObjectType type, const char* name);

private:
    VulkanContext() = default;
    ~VulkanContext() = default;

private:
    void CreateInstance(const char* app_name);
    void CreateSurface();
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateDebugMessenger();
    void CreateCommandPool();
    void CreateDescriptorPool();
    void CreateFrameContexts();
    void DestroyFrameContexts();
    void AdvanceFrame();
    void BuildFeatures();

private:
    ISurfaceProvider* surface_provider_{};
    VkInstance instance_{};
    VkPhysicalDevice physical_device_{};
    VkDevice device_{};
    VkQueue graphics_queue_{};
    uint32_t graphics_queue_family_index_{};
    uint32_t present_queue_family_index_{};
    VkPhysicalDeviceMemoryProperties memory_properties_{};
    VkPhysicalDeviceProperties device_properties_{};
    VkSurfaceKHR surface_{};
    VkCommandPool command_pool_{};
    VkDescriptorPool descriptor_pool_{};
    std::vector<FrameContext> frame_contexts_;
    std::unique_ptr<Swapchain> swapchain_;

    VkDebugUtilsMessengerEXT debug_messenger_{};
    PFN_vkSetDebugUtilsObjectNameEXT pfn_set_debug_utils_object_name_ext_{};

    uint32_t current_frame_index_{0};

    VkPhysicalDeviceFeatures2 physical_device_features_ {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
    };
    VkPhysicalDeviceVulkan11Features vulkan11_features_ {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
    };
    VkPhysicalDeviceVulkan12Features vulkan12_features_ {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
    };
    VkPhysicalDeviceVulkan13Features vulkan13_features_ {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
    };
    VkPhysicalDeviceShaderAtomicFloatFeaturesEXT atomic_float_features_ {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT,
    };
};
