#include "triangle_app.h"
#include "core/vulkan_context.h"
#include "core/sdl2_surface_provider.h"
#include "core/asset_path.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <iostream>
#include <string>
#include <filesystem>

int main(int argc, char *argv[]) {
	std::filesystem::path exe_dir = std::filesystem::path(argv[0]).parent_path();
	std::filesystem::current_path(exe_dir);
	std::filesystem::path asset_dir = exe_dir / "../../assets";
	SetAssetRootPath(asset_dir);

    SDL_Init(SDL_INIT_EVERYTHING);
    std::string window_name__("Example SDL2 Vulkan application");
    SDL_Window* window = SDL_CreateWindow(
            window_name__.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            1280, 720,
            SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN);
    Sdl2SurfaceProvider surface_provider(window);

    auto& vulkan_context = VulkanContext::Get();
    vulkan_context.GetWindowSystemExtensions = [window](auto& extensions) {
        uint32_t extension_count = 0;
        if (!SDL_Vulkan_GetInstanceExtensions(window, &extension_count, nullptr)) {
            throw std::runtime_error("Failed to get SDL Vulkan extensions");
        }
        std::vector<const char*> sdl_extensions(extension_count);
        if (!SDL_Vulkan_GetInstanceExtensions(window, &extension_count, sdl_extensions.data())) {
            throw std::runtime_error("Failed to get SDL Vulkan extensions");
        }
        extensions.insert(extensions.end(), sdl_extensions.begin(), sdl_extensions.end());
    };
    vulkan_context.Initialize("Kousoku Triangle", &surface_provider);
    vulkan_context.RecreateSwapchain();

    TriangleApp tri_app{};
    tri_app.Initialize();

    // Message Loop
    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        tri_app.DrawFrame();
    }

    SDL_DestroyWindow(window);
    window = nullptr;
    SDL_Quit();

    return 0;
}

