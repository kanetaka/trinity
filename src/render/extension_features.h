#pragma once
#include "render/vulkan_context.h"

namespace tr {





// Helper templates for Vulkan pNext chain construction
template<typename T>
void BuildVkExtensionChain(T& last)
{
    last.pNext = nullptr;
}
template<typename T, typename U, typename ... Rest>
void BuildVkExtensionChain(T& current, U& next, Rest&... rest)
{
    current.pNext = &next;
    BuildVkExtensionChain(next, rest...);
}

class IExtensionFeatureProvider
{
public:
    virtual ~IExtensionFeatureProvider() = default;

    // Returns a list of instance extensions to enable
    virtual void GetRequiredInstanceExtensions(std::vector<const char*>& extensionList) = 0;

    // Returns a list of device extensions to enable
    virtual void GetRequiredDeviceExtensions(std::vector<const char*>& extensionList) = 0;

    // Returns a pointer to be used for the pNext chain
    void* GetDeviceFeatures() {
        return reinterpret_cast<void*>(&phys_dev_features_);
    }

    // Constructs the extension link chain from phys_dev_features_
    virtual void BuildFeatures(VulkanContext* vulkanCtx) = 0;

protected:
    VkPhysicalDeviceFeatures2 phys_dev_features_{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
};


} // namespace tr
