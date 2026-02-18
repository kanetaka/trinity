#include "core/vulkan_context.h"
#include "core/swapchain.h"
#include <stdexcept>
#include <cassert>
#include <iostream>
#include <sstream>
#include <set>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif


VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data) {
    std::stringstream ss;
    ss << "[Validation Layer]" << callback_data->pMessage << std::endl;

#if defined(_WIN32)
    OutputDebugStringA(ss.str().c_str());
#else
    std::cerr << ss;
#endif
    return VK_FALSE;
}

// ------------------------------- //
// ---- Public Static Methods ---- //
// ------------------------------- //

VulkanContext& VulkanContext::Get() {
    static VulkanContext instance;
    return instance;
}

// ------------------------------- //
// ---- Public Member Methods ---- //
// ------------------------------- //

VulkanContext::FrameContext* VulkanContext::GetCurrentFrameContext() {
    return &frame_contexts_[current_frame_index_];
}

void VulkanContext::Initialize(const char* app_name, ISurfaceProvider* surface_provider) {
    surface_provider_ = surface_provider;
    
    CreateInstance(app_name);
    PickPhysicalDevice();
    CreateLogicalDevice();
    
#if DEBUG || _DEBUG
    CreateDebugMessenger();
#endif

    CreateCommandPool();
}

void VulkanContext::Cleanup() {
    vkDeviceWaitIdle(device_);
    DestroyFrameContexts();
    vkDestroyCommandPool(device_, command_pool_, nullptr);

    if (debug_messenger_ != VK_NULL_HANDLE) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance_, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance_, debug_messenger_, nullptr);
        }
        debug_messenger_ = VK_NULL_HANDLE;
    }

    if (swapchain_) {
        swapchain_->Cleanup();
        // TODO
    }

    if (surface_ != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(instance_, surface_, nullptr);
        surface_ = VK_NULL_HANDLE;
    }

    vkDestroyDevice(device_, nullptr);
    vkDestroyInstance(instance_, nullptr);
    device_ = VK_NULL_HANDLE;
    instance_ = VK_NULL_HANDLE;
}

void VulkanContext::RecreateSwapchain() {
    if (!swapchain_) {
        swapchain_ = std::make_unique<Swapchain>();
    }

    if (surface_ == VK_NULL_HANDLE) {
        CreateSurface();
    }

    auto width = surface_provider_->GetFrameBufferWidth();
    auto height = surface_provider_->GetFrameBufferHeight();
    swapchain_->Recreate(width, height);

    DestroyFrameContexts();
    CreateFrameContexts();
}

std::shared_ptr<CommandBuffer> VulkanContext::CreateCommandBuffer() {
    VkCommandBufferAllocateInfo command_alloc_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = command_pool_,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VkCommandBuffer command_buffer{};
    vkAllocateCommandBuffers(device_, &command_alloc_info, &command_buffer);

    return std::make_shared<CommandBuffer>(command_buffer);
}

VkResult VulkanContext::AcquireNextImage() {
    auto* frame = GetCurrentFrameContext();
    auto fence = frame->inflight_fence;
    vkWaitForFences(device_, 1, &fence, VK_TRUE, UINT64_MAX);

    auto result = swapchain_->AcquireNextImage();
    if (result == VK_SUCCESS) {
        vkResetFences(device_, 1, &fence);
    }
    else if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        auto width = surface_provider_->GetFrameBufferWidth();
        auto height = surface_provider_->GetFrameBufferHeight();
        if (width > 0 && height > 0) {
            swapchain_->Recreate(width, height);
        }
    }
    assert(result != VK_ERROR_DEVICE_LOST);

    return result;
}

void VulkanContext::SubmitPresent() {
    auto& frame = frame_contexts_[GetCurrentFrameIndex()];

    VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSemaphore render_complete_semaphore = swapchain_->GetRenderCompleteSemaphore();
    VkSemaphore present_complete_semaphore = swapchain_->GetPresentCompleteSemaphore();
    VkCommandBuffer command_buffer = frame.command_buffer->Get();

    VkSubmitInfo submit_info {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &present_complete_semaphore,
        .pWaitDstStageMask = &wait_stage_mask,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &render_complete_semaphore,
    };
    auto result = vkQueueSubmit(graphics_queue_, 1, &submit_info, frame.inflight_fence);
    assert(result != VK_ERROR_DEVICE_LOST);

    swapchain_->QueuePresent(graphics_queue_);
    AdvanceFrame();
}

uint32_t VulkanContext::FindMemoryType(const VkMemoryRequirements& requirements, VkMemoryPropertyFlags propeties) const {
    for (uint32_t i = 0; i < memory_properties_.memoryTypeCount; ++i) {
        const bool is_type_compatible = (requirements.memoryTypeBits & (1 << i)) != 0;
        const bool has_desired_properties = (memory_properties_.memoryTypes[i].propertyFlags & propeties) == propeties;
        if (is_type_compatible && has_desired_properties) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type");
}

void VulkanContext::SetDebugObjectName(void* object_handle, VkObjectType type, const char* name) {
#if _DEBUG || DEBUG
    if (pfn_set_debug_utils_object_name_ext_) {
        VkDebugUtilsObjectNameInfoEXT name_info {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = type,
            .objectHandle = reinterpret_cast<uint64_t>(object_handle),
            .pObjectName = name,
        };
        pfn_set_debug_utils_object_name_ext_(device_, &name_info);
    }
#endif // _DEBUG || DEBUG
}

// -------------------------------- //
// ---- Private Member Methods ---- //
// -------------------------------- //

void VulkanContext::CreateInstance(const char* app_name) {
    VkApplicationInfo app_info {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = app_name,
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "Kousoku Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_3,
    };

    std::vector<const char*> extensions;
    std::vector<const char*> layers;

#if DEBUG || _DEBUG
    // Enable debug utils extension
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    // Enable validation layer
    layers.push_back("VK_LAYER_KHRONOS_validation");
#endif // DEBUG || _DEBUG

    GetWindowSystemExtensions(extensions);

    VkInstanceCreateInfo instance_info {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = uint32_t(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = uint32_t(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    if (vkCreateInstance(&instance_info, nullptr, &instance_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan instance");
    }
}

void VulkanContext::CreateSurface() {
    surface_ = surface_provider_->CreateSurface(instance_);
    if (surface_ == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to create Vulkan surface from surface provider");
    }

    VkBool32 present_support = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device_, graphics_queue_family_index_, surface_, &present_support);
    if (present_support == VK_FALSE) {
        throw std::runtime_error("Selected physical device does not support presentation");
    }
}


void VulkanContext::PickPhysicalDevice() {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance_, &device_count, nullptr);
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance_, &device_count, devices.data());
    physical_device_ = devices[0]; // TODO Select proper device

    // Get device properties
    vkGetPhysicalDeviceMemoryProperties(physical_device_, &memory_properties_);
    vkGetPhysicalDeviceProperties(physical_device_, &device_properties_);
}

void VulkanContext::CreateLogicalDevice() {
    // Queue family settings
    uint32_t queue_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &queue_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &queue_count, queue_families.data());

    graphics_queue_family_index_ = ~0u;
    for (uint32_t i = 0; const auto& props: queue_families) {
        if (props.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphics_queue_family_index_ = i;
            break;
        }
        ++i;
    }

    // Extension settings
    BuildFeatures();
    std::vector<const char*> device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    float priority = 1.0f;
    VkDeviceQueueCreateInfo queue_info {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = graphics_queue_family_index_,
        .queueCount = 1,
        .pQueuePriorities = &priority,
    };

    VkDeviceCreateInfo device_info {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &physical_device_features_,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queue_info,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = uint32_t(device_extensions.size()),
        .ppEnabledExtensionNames = device_extensions.data(),
        .pEnabledFeatures = nullptr, // Using pNext chain instead
    };

    auto result = vkCreateDevice(physical_device_, &device_info, nullptr, &device_);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device");
    }
    vkGetDeviceQueue(device_, graphics_queue_family_index_, 0, &graphics_queue_);
}

void VulkanContext::CreateCommandPool() {
    VkCommandPoolCreateInfo cmd_pool_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = graphics_queue_family_index_
    };
    
    if (vkCreateCommandPool(device_, &cmd_pool_info, nullptr, &command_pool_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool");
    }
}

void VulkanContext::CreateFrameContexts() {
    frame_contexts_.resize(MaxInflightFrames);
    for (auto& frame: frame_contexts_) {
        frame.command_buffer = CreateCommandBuffer();
        VkFenceCreateInfo fence_info{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };
        vkCreateFence(device_, &fence_info, nullptr, &frame.inflight_fence);
    }
}

void VulkanContext::DestroyFrameContexts() {
    for (auto& frame: frame_contexts_) {
        vkDestroyFence(device_, frame.inflight_fence, nullptr);
    }
    frame_contexts_.clear();
}

void VulkanContext::AdvanceFrame() {
    current_frame_index_ = (current_frame_index_ + 1) % MaxInflightFrames;
}

template<typename T>
void BuildExtensionChain(T& last) {
    last.pNext = nullptr;
}

template<typename T, typename U, typename... Rest>
void BuildExtensionChain(T& current, U& next, Rest&... rest) {
    current.pNext = &next;
    BuildExtensionChain(next, rest...);
}

void VulkanContext::BuildFeatures() {
    BuildExtensionChain(
        physical_device_features_,
        vulkan11_features_,
        vulkan12_features_,
        vulkan13_features_
        );

    // Support status acquisition
    vkGetPhysicalDeviceFeatures2(physical_device_, &physical_device_features_);

    // Enable features as needed
    vulkan13_features_.dynamicRendering = VK_TRUE;
    vulkan13_features_.synchronization2 = VK_TRUE;
}

void VulkanContext::CreateDebugMessenger() {
    VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = nullptr,
        .flags = 0,
        .messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = DebugCallback,
        .pUserData = nullptr,
    };
}
