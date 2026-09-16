#pragma once


#ifdef VK_USE_PLATFORM_WIN32_KHR
# define NOMINMAX
#endif
#include <vulkan/vulkan.h>
#include <vector>

namespace tri
{
    class ISurfaceProvider
    {
    public:
        virtual ~ISurfaceProvider() = default;
        virtual VkSurfaceKHR CreateSurface(VkInstance instance) = 0;
        virtual void GetRequiredExtensions(std::vector<const char*>& extensions) const = 0;
        virtual uint32_t GetFramebufferWidth() const = 0;
        virtual uint32_t GetFramebufferHeight() const = 0;
    };
} // namespace tri
