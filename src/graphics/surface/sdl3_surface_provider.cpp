#include "graphics/surface/sdl3_surface_provider.h"
#include <SDL3/SDL_vulkan.h>
#include <iostream>
#include <stdexcept>

using namespace tri;


Sdl3SurfaceProvider::Sdl3SurfaceProvider(SDL_Window* window)
        : window_(window) {}

VkSurfaceKHR Sdl3SurfaceProvider::CreateSurface(VkInstance instance)
{
    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(window_, instance, nullptr, &surface))
    {
        throw std::runtime_error("failed to create window surface!");
    }
    return surface;
}

void Sdl3SurfaceProvider::GetRequiredExtensions(std::vector<const char*>& extensions) const
{
    uint32_t ext_count = 0;
    char const* const* sdl_extensions = SDL_Vulkan_GetInstanceExtensions(&ext_count);
    if (ext_count > 0 && sdl_extensions != nullptr)
    {
        for (uint32_t i = 0; i < ext_count; ++i)
        {
            extensions.push_back(sdl_extensions[i]);
        }
    }
}

uint32_t Sdl3SurfaceProvider::GetFramebufferWidth() const
{
    int width, height;
    SDL_GetWindowSizeInPixels(window_, &width, &height);
    return static_cast<uint32_t>(width);
}

uint32_t Sdl3SurfaceProvider::GetFramebufferHeight() const
{
    int width, height;
    SDL_GetWindowSizeInPixels(window_, &width, &height);
    return static_cast<uint32_t>(height);
}
