#pragma once

#include <SDL2/SDL.h>
#include <string>
#include "vulkan_extern.h"
#include "vulkan_function.h"

struct SDL_Window;
class Vulkan;

namespace x8 {
class Renderer {
private:
	SDL_Window* window_;
	std::string window_name_ = "Example SDL2 Vulkan application";
	Vulkan* vulkan_;

public:
	Renderer();
	~Renderer();

	void InitVulkan();
	void Run();

private: 
	void init_vulkan_extern();
};
} // namespace x8
