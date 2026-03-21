#include "render/vulkan_context.h"
#include "render/swapchain.h"



#if defined(__ANDROID__)
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>
#endif

#include <cassert>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace trinity::render {

#endif

#define VK_GET_INSTANCE_PROC_ADDR(instance, name, ...)                         \
  reinterpret_cast<PFN_##name>(vkGetInstanceProcAddr(instance, #name))

VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    std::stringstream ss;
    ss << "[Validation Layer] " << pCallbackData->pMessage << std::endl;

#if defined(_WIN32)
    OutputDebugStringA(ss.str().c_str());
#else
    std::cerr << ss.str() << std::endl;
#endif
    return VK_FALSE;
}

VulkanContext& VulkanContext::Get()
{
    static VulkanContext instance;
    return instance;
}

void VulkanContext::Initialize(const char* app_name,
    ISurfaceProvider* surface_provider)
{
    surface_provider_ = surface_provider;
    CreateInstance(app_name);

    PickPhysicalDevice();
    CreateDebugMessenger();
    CreateLogicalDevice();

    CreateCommandPool();
    CreateDescriptorPool();
}

void VulkanContext::Cleanup()
{
    // Wait for the device to be idle before proceeding with cleanup
    vkDeviceWaitIdle(vk_device_);

    DestroyFrameContexts();
    vkDestroyCommandPool(vk_device_, command_pool_, nullptr);
    vkDestroyDescriptorPool(vk_device_, descriptor_pool_, nullptr);

    if (debug_messenger_ != VK_NULL_HANDLE)
    {
        auto func = VK_GET_INSTANCE_PROC_ADDR(vk_instance_, vkDestroyDebugUtilsMessengerEXT);
        if (func != nullptr)
        {
            func(vk_instance_, debug_messenger_, nullptr);
        }
        debug_messenger_ = VK_NULL_HANDLE;
    }

    if (swapchain_)
    {
        swapchain_->Cleanup();
        swapchain_.reset();
    }

    if (surface_ != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(vk_instance_, surface_, nullptr);
        surface_ = VK_NULL_HANDLE;
    }
    vkDestroyDevice(vk_device_, nullptr);
    vkDestroyInstance(vk_instance_, nullptr);

    vk_device_ = VK_NULL_HANDLE;
    vk_instance_ = VK_NULL_HANDLE;
}

void VulkanContext::RecreateSwapchain()
{
    if (swapchain_ == nullptr)
    {
        swapchain_ = std::make_unique<Swapchain>();
    }

    if (surface_ == VK_NULL_HANDLE)
    {
        CreateSurface();
    }

    auto width = surface_provider_->GetFramebufferWidth();
    auto height = surface_provider_->GetFramebufferHeight();
    swapchain_->Recreate(width, height);

    DestroyFrameContexts();
    CreateFrameContexts();
}

VkResult VulkanContext::AcquireNextImage()
{
    auto* frame = GetCurrentFrameContext();
    auto fence = frame->inflightFence;
    vkWaitForFences(vk_device_, 1, &fence, VK_TRUE, UINT64_MAX);

    auto result = swapchain_->AcquireNextImage();
    if (result == VK_SUCCESS)
    {
        vkResetFences(vk_device_, 1, &fence);
    }
    else if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        // Handle window minimization
        auto width = surface_provider_->GetFramebufferWidth();
        auto height = surface_provider_->GetFramebufferHeight();
        if (width > 0 && height > 0)
        {
            swapchain_->Recreate(width, height);
        }
    }
    assert(result != VK_ERROR_DEVICE_LOST);
    return result;
}

void VulkanContext::SubmitPresent()
{
    auto& frame = frame_context_[GetCurrentFrameIndex()];

    VkPipelineStageFlags wait_stage_mask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info{ .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO };
    VkSemaphore render_complete_sem = swapchain_->GetRenderCompleteSemaphore();
    VkSemaphore present_complete_sem = swapchain_->GetPresentCompleteSemaphore();

    VkCommandBuffer command_buffer = frame.commandBuffer->Get();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    submit_info.pWaitDstStageMask = &wait_stage_mask;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &present_complete_sem;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &render_complete_sem;
    auto result =
        vkQueueSubmit(graphics_queue_, 1, &submit_info, frame.inflightFence);
    assert(result != VK_ERROR_DEVICE_LOST);

    // Graphics queue has already been checked for presentation support
    swapchain_->QueuePresent(graphics_queue_);
    AdvanceFrame();
}

void VulkanContext::SubmitAndWait(
    std::shared_ptr<CommandBuffer> command_buffer)
{
    auto command_buf = command_buffer->Get();
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buf;

    VkFence fence;
    VkFenceCreateInfo create_info
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };
    vkCreateFence(vk_device_, &create_info, nullptr, &fence);
    vkQueueSubmit(graphics_queue_, 1, &submit_info, fence);
    vkWaitForFences(vk_device_, 1, &fence, VK_TRUE, UINT64_MAX);

    vkDestroyFence(vk_device_, fence, nullptr);
}

VulkanContext::FrameContext* VulkanContext::GetCurrentFrameContext()
{
    return &frame_context_[current_frame_index_];
}

uint32_t VulkanContext::FindMemoryType(const VkMemoryRequirements& requirements,
    VkMemoryPropertyFlags properties) const {
    for (uint32_t i = 0; i < memory_properties_.memoryTypeCount; i++)
    {
        const bool is_type_compatible =
            (requirements.memoryTypeBits & (1 << i)) != 0;
        const bool has_desired_properties =
            (memory_properties_.memoryTypes[i].propertyFlags & properties) ==
            properties;

        if (is_type_compatible && has_desired_properties)
        {
            // Memory type matches properties and is included in memoryTypeBits
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

uint32_t VulkanContext::MinUniformOffsetAlignment() const
{
    const auto& limits = physical_device_properties_.limits;
    return limits.minUniformBufferOffsetAlignment;
}

uint32_t VulkanContext::MinStorageBufferOffsetAlignment() const
{
    const auto& limits = physical_device_properties_.limits;
    return limits.minStorageBufferOffsetAlignment;
    ;
}

uint32_t VulkanContext::NonCoherentAtomSize() const
{
    const auto& limits = physical_device_properties_.limits;
    return limits.nonCoherentAtomSize;
}

void VulkanContext::SetDebugObjectName(void* object_handle, VkObjectType type,
    const char* name)
{
#if _DEBUG || DEBUG
    if (pfn_set_debug_utils_object_name_ext_)
    {
        VkDebugUtilsObjectNameInfoEXT name_info
        {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .objectType = type,
            .objectHandle = reinterpret_cast<uint64_t>(object_handle),
            .pObjectName = name,
        };
        pfn_set_debug_utils_object_name_ext_(vk_device_, &name_info);
    }
#endif
}

void VulkanContext::CreateInstance(const char* app_name)
{
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = app_name;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "VulkanBookEngine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_3;

    std::vector<const char*> extension_list;
    std::vector<const char*> layer_list;

#if DEBUG || _DEBUG
    // Enable VK_EXT_debug_utils for development
    extension_list.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    // Enable validation layers for development
    layer_list.push_back("VK_LAYER_KHRONOS_validation");
#endif
    GetWindowSystemExtensions(extension_list);

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = uint32_t(extension_list.size());
    create_info.ppEnabledExtensionNames = extension_list.data();
    create_info.enabledLayerCount = uint32_t(layer_list.size());
    create_info.ppEnabledLayerNames = layer_list.data();

    if (vkCreateInstance(&create_info, nullptr, &vk_instance_) != VK_SUCCESS)
        throw std::runtime_error("failed to create instance");
}

void VulkanContext::CreateSurface()
{
    surface_ = surface_provider_->CreateSurface(vk_instance_);

    // Check if the graphics queue family supports presentation to the surface
    VkBool32 present = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(
        vk_physical_device_, graphics_queue_family_index_, surface_, &present);
    if (present == VK_FALSE)
    {
        throw std::runtime_error("not supported presentation");
    }
}

void VulkanContext::PickPhysicalDevice()
{
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(vk_instance_, &count, nullptr);
    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(vk_instance_, &count, devices.data());
    vk_physical_device_ = devices[0];

    vkGetPhysicalDeviceMemoryProperties(vk_physical_device_, &memory_properties_);
    vkGetPhysicalDeviceProperties(vk_physical_device_,
        &physical_device_properties_);
}

void VulkanContext::CreateLogicalDevice()
{
    // Find graphics queue family index
    uint32_t queue_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device_, &queue_count,
        nullptr);
    std::vector<VkQueueFamilyProperties> queues(queue_count);
    vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device_, &queue_count,
        queues.data());

    graphics_queue_family_index_ = ~0u;
    for (uint32_t i = 0; const auto& props : queues)
    {
        if (props.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphics_queue_family_index_ = i;
            break;
        }
        ++i;
    }

    BuildVkFeatures();
    std::vector<const char*> device_extensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    // Enable for Y-flip (VK_KHR_maintenance1)
    device_extensions.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);

#if defined(CHAPTER_COMPUTE_SHADER)
    // Add extension if shaderBufferFloat32AtomicAdd is supported
    if (atomic_float_features_.shaderBufferFloat32AtomicAdd)
    {
        device_extensions.push_back(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
    }
#endif

    float priority = 1.0f;
    VkDeviceQueueCreateInfo queue_info{};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.queueFamilyIndex = graphics_queue_family_index_;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &priority;

    VkDeviceCreateInfo device_info{};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.queueCreateInfoCount = 1;
    device_info.pQueueCreateInfos = &queue_info;
    device_info.enabledExtensionCount = uint32_t(device_extensions.size());
    device_info.ppEnabledExtensionNames = device_extensions.data();

    device_info.pNext = &phys_dev_features_;
    device_info.pEnabledFeatures = nullptr;

    auto result =
        vkCreateDevice(vk_physical_device_, &device_info, nullptr, &vk_device_);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device");
    }

    vkGetDeviceQueue(vk_device_, graphics_queue_family_index_, 0,
        &graphics_queue_);

#if _DEBUG
    SetDebugObjectName(vk_device_, VK_OBJECT_TYPE_DEVICE, "MainVkDevice");
#endif
}

void VulkanContext::CreateDebugMessenger()
{
    VkDebugUtilsMessengerCreateInfoEXT create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = VulkanDebugCallback;

    auto vk_create_debug_utils_messenger =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            vk_instance_, "vkCreateDebugUtilsMessengerEXT");

    if (vk_create_debug_utils_messenger &&
        vk_create_debug_utils_messenger(vk_instance_, &create_info, nullptr,
            &debug_messenger_) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }

    pfn_set_debug_utils_object_name_ext_ =
        (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(
            vk_instance_, "vkSetDebugUtilsObjectNameEXT");
}

void VulkanContext::CreateCommandPool()
{
    VkCommandPoolCreateInfo command_pool_ci
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    };
    command_pool_ci.queueFamilyIndex = graphics_queue_family_index_;
    command_pool_ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(vk_device_, &command_pool_ci, nullptr, &command_pool_);
}

void VulkanContext::CreateDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> pool_sizes =
    {
        {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 4096 },
        {.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 4096 },
        {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, .descriptorCount = 4096 },
        {.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 4096 },
        {.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .descriptorCount = 4096 },
    };

    VkDescriptorPoolCreateInfo pool_info
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 4096,
        .poolSizeCount = uint32_t(pool_sizes.size()),
        .pPoolSizes = pool_sizes.data(),
    };

    if (vkCreateDescriptorPool(vk_device_, &pool_info, nullptr,
        &descriptor_pool_) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void VulkanContext::CreateFrameContexts()
{
    frame_context_.resize(MaxInflightFrames);
    for (auto& frame : frame_context_)
    {
        frame.commandBuffer = CreateCommandBuffer();

        VkFenceCreateInfo fence_ci
        {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };
        vkCreateFence(vk_device_, &fence_ci, nullptr, &frame.inflightFence);
    }
}

void VulkanContext::DestroyFrameContexts()
{
    for (auto& frame : frame_context_)
    {
        vkDestroyFence(vk_device_, frame.inflightFence, nullptr);
    }
    frame_context_.clear();
}

void VulkanContext::AdvanceFrame()
{
    current_frame_index_ = (current_frame_index_ + 1) % MaxInflightFrames;
}

// Helper template to simplify building the pNext chain for Vulkan structs
template <typename T> void BuildVkExtensionChain(T& last)
{
    last.pNext = nullptr;
}

template <typename T, typename U, typename... Rest>
void BuildVkExtensionChain(T& current, U& next, Rest &...rest)
{
    current.pNext = &next;
    BuildVkExtensionChain(next, rest...);
}

void VulkanContext::BuildVkFeatures()
{
    // Enable desired features after getting support info from the device.
    // Enabling unsupported features will cause errors during device creation.
#if !defined(CHAPTER_COMPUTE_SHADER)
    BuildVkExtensionChain(phys_dev_features_, vulkan11_features_,
        vulkan12_features_, vulkan13_features_);
    // Get support status
    vkGetPhysicalDeviceFeatures2(vk_physical_device_, &phys_dev_features_);

    vulkan13_features_.dynamicRendering = VK_TRUE;
    vulkan13_features_.synchronization2 = VK_TRUE;
#endif

#if defined(CHAPTER_COMPUTE_SHADER)
    BuildVkExtensionChain(phys_dev_features_, vulkan11_features_,
        vulkan12_features_, vulkan13_features_,
        atomic_float_features_);
    // Get support status
    vkGetPhysicalDeviceFeatures2(vk_physical_device_, &phys_dev_features_);

    vulkan13_features_.dynamicRendering = VK_TRUE;
    vulkan13_features_.synchronization2 = VK_TRUE;
#endif

    // Some environments (APUs) may warn if robustBufferAccess=true
    // - Disable it as it's not needed for this sample
    phys_dev_features_.features.robustBufferAccess = VK_FALSE;
}

std::shared_ptr<CommandBuffer> VulkanContext::CreateCommandBuffer()
{
    VkCommandBufferAllocateInfo command_ai
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = command_pool_,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VkCommandBuffer command_buffer{};
    vkAllocateCommandBuffers(vk_device_, &command_ai, &command_buffer);

    return std::make_shared<CommandBuffer>(command_buffer);
}

VkDescriptorSet
VulkanContext::AllocateDescriptorSet(VkDescriptorSetLayout layout)
{
    VkDescriptorSetAllocateInfo alloc_info
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descriptor_pool_,
        .descriptorSetCount = 1,
        .pSetLayouts = &layout,
    };

    VkDescriptorSet descriptor_set;
    if (vkAllocateDescriptorSets(vk_device_, &alloc_info, &descriptor_set) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor set!");
    }

    return descriptor_set;
}

void VulkanContext::FreeDescriptorSet(VkDescriptorSet descriptor_set)
{
    vkFreeDescriptorSets(vk_device_, descriptor_pool_, 1, &descriptor_set);
}


} // namespace trinity::render
