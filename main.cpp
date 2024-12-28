// g++ *.cpp -o vulkan -lSDL2main -lSDL2 -lvulkan-1
// https://vulkan-tutorial.com/

#include <iostream>
#include <string>
using namespace std;

#include <SDL2/SDL.h>
SDL_Window* window__;
std::string window_name__("Example SDL2 Vulkan application");

#include "vulkan_extern.h"
#include "vulkan_function.h"
Vulkan* vulkan;

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_EVERYTHING);
    window__ = SDL_CreateWindow(window_name__.c_str(), SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,800,600,SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN);

    vulkan = new Vulkan();
    initVulkanExtern(vulkan);

    SDL_Event event;
    bool running = true;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        acquireNextImage();
        resetCommandBuffer();
        beginCommandBuffer();
        {
            VkClearColorValue clear_color = {0.0f, 0.0f, 0.8f, 1.0f};
            VkClearDepthStencilValue clear_depth_stencil = {1.0f, 0};
            beginRenderPass(clear_color, clear_depth_stencil);
            {
            }
            endRenderPass();
        }
        endCommandBuffer();

        queueSubmit();
        queuePresent();
    }

    delete vulkan;
    vulkan = nullptr;

    SDL_DestroyWindow(window__);
    window__ = nullptr;
    SDL_Quit();

    return 0;
}
