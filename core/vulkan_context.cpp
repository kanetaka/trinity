#include "vulkan_context.h"
#include "swapchain.h"

#include <stdexcept>
#include <cassert>
#include <iostream>>
#include <sstream>
#include <set>

VulkanContext& VulkanContext::Get() {
    static VulkanContext instance;
    return instance;
}

void VulkanContext::Initialize(const char* app_name, ISurfaceProvider* surface_provider) {
    surface_provider_ = surface_provider;
} // VulkanContext::Initialize

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

    // TODO Debug layers

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
} // VulkanContext::CreateInstance

void VulkanContext::PickPhysicalDevice() {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance_, &device_count, nullptr);
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance_, &device_count, devices.data());
    physical_device_ = devices[0]; // TODO Select proper device

    // Get device properties
    vkGetPhysicalDeviceMemoryProperties(physical_device_, &memory_properties_);
    vkGetPhysicalDeviceProperties(physical_device_, &device_properties_);
} // VulkanContext::PickPhysicalDevice

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
} // VulkanContext::CreateLogicalDevice

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
} // VulkanContext::BuildFeatures
