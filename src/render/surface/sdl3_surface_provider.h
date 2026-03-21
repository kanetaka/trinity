#pragma once
#include "render/surface/surface_provider.h"
#include <SDL3/SDL.h>

class Sdl3SurfaceProvider : public ISurfaceProvider
{
public:
    explicit Sdl3SurfaceProvider(SDL_Window* window);
    VkSurfaceKHR CreateSurface(VkInstance instance) override;
    uint32_t GetFramebufferWidth() const override;
    uint32_t GetFramebufferHeight() const override;

private:
    SDL_Window* window_;
};
