#include "sdl3_surface_provider.h"
#include <SDL3/SDL_vulkan.h>
#include <iostream>
#include <stdexcept>

Sdl3SurfaceProvider::Sdl3SurfaceProvider(SDL_Window *window)
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
