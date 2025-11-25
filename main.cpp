#include "triangle_app.h"
#include "vulkan_context.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_EVERYTHING);
    std::string window_name__("Example SDL2 Vulkan application");
    SDL_Window* window = SDL_CreateWindow(window_name__.c_str(), SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,800,600,SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN);

    auto& vulkan_ctx = VulkanContext::Get();
    TriangleApp app{};
    app.OnInitialize();

    // メッセージループ
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
