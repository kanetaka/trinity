#include "core/sdl2_surface_provider.h"

#include <SDL2/SDL_vulkan.h>
#include <stdexcept>

#include <iostream>

Sdl2SurfaceProvider::Sdl2SurfaceProvider(SDL_Window* window)
	: window_(window) {
}

VkSurfaceKHR Sdl2SurfaceProvider::CreateSurface(VkInstance instance) {
	VkSurfaceKHR surface;
	if (!SDL_Vulkan_CreateSurface(window_, instance, &surface)) {
		throw std::runtime_error("Failed to create Vulkan surface from SDL2 window");
	}
	return surface;
}

uint32_t Sdl2SurfaceProvider::GetFrameBufferWidth() const {
	int width = 0;
	SDL_Vulkan_GetDrawableSize(window_, &width, nullptr);
	return static_cast<uint32_t>(width);
}

uint32_t Sdl2SurfaceProvider::GetFrameBufferHeight() const {
	int height = 0;
	SDL_Vulkan_GetDrawableSize(window_, nullptr, &height);
	return static_cast<uint32_t>(height);
}
