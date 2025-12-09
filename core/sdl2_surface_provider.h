#pragma once

#include "surface_provider.h"
#include <SDL2/SDL.h>

class Sdl2SurfaceProvider : public ISurfaceProvider {
public:
	explicit Sdl2SurfaceProvider(SDL_Window* window);
	VkSurfaceKHR  CreateSurface(VkInstance instance) override;
	uint32_t GetFrameBufferWidth() const override;
	uint32_t GetFrameBufferHeight() const override;

private:
	SDL_Window* window_;
};
