#include "triangle_app.h"
#include "core/vulkan_context.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_EVERYTHING);
    std::string window_name__("Example SDL2 Vulkan application");
    SDL_Window* window = SDL_CreateWindow(window_name__.c_str(), SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,800,600,SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN);

    auto& vulkan_ctx = VulkanContext::Get();
    vulkan_ctx.GetWindowSystemExtensions = [window](auto& extensions) {
        unsigned int extension_count = 0;
        if (!SDL_Vulkan_GetInstanceExtensions(window, &extension_count, nullptr)) {
            throw std::runtime_error("Failed to get SDL Vulkan extensions");
        }
        std::vector<const char*> sdl_extensions(extension_count);
        if (!SDL_Vulkan_GetInstanceExtensions(window, &extension_count, sdl_extensions.data())) {
            throw std::runtime_error("Failed to get SDL Vulkan extensions");
        }
        extensions.insert(extensions.end(), sdl_extensions.begin(), sdl_extensions.end());
	};

    TriangleApp app{};
    app.OnInitialize();

    // Message Loop
    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        app.OnDrawFrame();
    }

    SDL_DestroyWindow(window);
    window = nullptr;
    SDL_Quit();

    return 0;
}

