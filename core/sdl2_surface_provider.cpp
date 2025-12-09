#include "core/sdl2_surface_provider.h"
#include <SDL2/SDL_vulkan.h>
#include <stdexcept>
#include <iostream>

Sdl2SurfaceProvider::Sdl2SurfaceProvider(SDL_Window* window)
	: window_(window) {
}

VkSurfaceKHR Sdl2SurfaceProvider::CreateSurface(VkInstance instance) {
	if (!window_) {
		std::cerr << "Error: SDL_Window is null" << std::endl;
		throw std::runtime_error("SDL_Window is null");
	}
	
	if (instance == VK_NULL_HANDLE) {
		std::cerr << "Error: VkInstance is null" << std::endl;
		throw std::runtime_error("VkInstance is null");
	}

	VkSurfaceKHR surface;
	if (!SDL_Vulkan_CreateSurface(window_, instance, &surface)) {
		const char* sdl_error = SDL_GetError();
		std::cerr << "SDL_Vulkan_CreateSurface failed: " << sdl_error << std::endl;
		std::cerr << "Possible causes:" << std::endl;
		std::cerr << "1. Window was not created with SDL_WINDOW_VULKAN flag" << std::endl;
		std::cerr << "2. Required Vulkan instance extensions are missing" << std::endl;
		std::cerr << "3. Vulkan runtime or drivers are not properly installed" << std::endl;
		
		throw std::runtime_error(std::string("Failed to create Vulkan surface: ") + sdl_error);
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
