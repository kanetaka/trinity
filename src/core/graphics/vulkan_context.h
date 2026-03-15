#pragma once
#include <cstring>
#include <functional>
#include <memory>
#include <stdint.h>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "core/graphics/command_buffer.h"

class Swapchain;
class CommandBuffer;
class ISurfaceProvider;

class VulkanContext
{
public:
  static constexpr uint32_t MaxInflightFrames = 2;
  static VulkanContext &Get();

  // Initialization
  void Initialize(const char *appName, ISurfaceProvider *surfaceProvider);

  // Cleanup
  void Cleanup();

  // Create/Recreate swapchain
  void RecreateSwapchain();

  // Get various Vulkan objects
  VkInstance GetVkInstance() const { return vk_instance_; }
  VkDevice GetVkDevice() const { return vk_device_; }
  VkPhysicalDevice GetVkPhysicalDevice() const { return vk_physical_device_; }
  VkDescriptorPool GetVkDescriptorPool() const { return descriptor_pool_; }

  VkQueue GetGraphicsQueue() const { return graphics_queue_; }
  uint32_t GetGraphicsFamily() const { return graphics_queue_family_index_; }
  uint32_t GetPresentFamily() const { return present_queue_family_index_; }

  VkCommandPool GetCommandPool() const { return command_pool_; }
  VkSurfaceKHR GetSurface() const { return surface_; }

  // Create command buffer
  std::shared_ptr<CommandBuffer> CreateCommandBuffer();

  // Allocate descriptor set
  VkDescriptorSet AllocateDescriptorSet(VkDescriptorSetLayout layout);
  // Free descriptor set
  void FreeDescriptorSet(VkDescriptorSet descriptorSet);

  // Context information handled per rendering frame
  struct FrameContext {
    std::shared_ptr<CommandBuffer> commandBuffer;
    VkFence inflightFence = VK_NULL_HANDLE;
  };
  // Get current frame index
  uint32_t GetCurrentFrameIndex() const { return current_frame_index_; }
  // Acquire next available swapchain image
  VkResult AcquireNextImage();

  // Submit command buffer and present image
  void SubmitPresent();

  // Submit specified command buffer and wait for completion
  void SubmitAndWait(std::shared_ptr<CommandBuffer> commandBuffer);

  // Get current frame context
  FrameContext *GetCurrentFrameContext();

  // Get swapchain
  std::unique_ptr<Swapchain> &GetSwapchain() { return swapchain_; }

  // Find suitable memory type index
  uint32_t FindMemoryType(const VkMemoryRequirements &requirements,
                          VkMemoryPropertyFlags properties) const;

  // Minimum alignment for uniform buffer offsets
  uint32_t MinUniformOffsetAlignment() const;

  // Minimum alignment for storage buffer offsets
  uint32_t MinStorageBufferOffsetAlignment() const;

  // Minimum synchronization alignment for non-coherent memory between CPU and
  // GPU
  uint32_t NonCoherentAtomSize() const;

  // Function Callback(s)
  std::function<void(std::vector<const char *> &)> GetWindowSystemExtensions;

  // Set debug name for Vulkan object
  void SetDebugObjectName(void *objectHandle, VkObjectType type,
                          const char *name);

private:
  VulkanContext() = default;
  ~VulkanContext() = default;

private:
  void CreateInstance(const char *appName);
  void CreateSurface();
  void PickPhysicalDevice();
  void CreateLogicalDevice();
  void CreateDebugMessenger();
  void CreateCommandPool();
  void CreateDescriptorPool();

  void CreateFrameContexts();
  void DestroyFrameContexts();

  void AdvanceFrame();
  void BuildVkFeatures();

  ISurfaceProvider *surface_provider_{};
  VkInstance vk_instance_{};

  VkPhysicalDevice vk_physical_device_{};
  VkDevice vk_device_{};
  VkQueue graphics_queue_{};
  uint32_t graphics_queue_family_index_{};
  uint32_t present_queue_family_index_{};
  VkPhysicalDeviceMemoryProperties memory_properties_{};
  VkPhysicalDeviceProperties physical_device_properties_{};

  VkSurfaceKHR surface_{};
  VkCommandPool command_pool_{};
  VkDescriptorPool descriptor_pool_{};
  std::vector<FrameContext> frame_context_;
  std::unique_ptr<Swapchain> swapchain_;

  VkDebugUtilsMessengerEXT debug_messenger_{};
  PFN_vkSetDebugUtilsObjectNameEXT pfn_set_debug_utils_object_name_ext_{};

  uint32_t current_frame_index_ = 0;

  // --------
  VkPhysicalDeviceFeatures2 phys_dev_features_{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
  VkPhysicalDeviceVulkan11Features vulkan11_features_{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
  VkPhysicalDeviceVulkan12Features vulkan12_features_{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
  VkPhysicalDeviceVulkan13Features vulkan13_features_{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
  VkPhysicalDeviceShaderAtomicFloatFeaturesEXT atomic_float_features_{
      .sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT};
};
