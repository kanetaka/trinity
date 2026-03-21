#if defined(_WIN32)
#include <windows.h>
#endif

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "render/surface/sdl3_surface_provider.h"
#include "render/swapchain.h"
#include "render/vulkan_context.h"

#include "core/asset_path.h"
#include "render/resources/buffer_resource.h"

#include "glm/glm.hpp"
#include "app/application.h"

namespace fs = std::filesystem;

int RunApplication()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL_Init failed" << std::endl;
        return -1;
    }

    SDL_Window *window = nullptr;

    try
    {
        window = SDL_CreateWindow("Trinity",
                1280, 720,
                SDL_WINDOW_VULKAN | SDL_WINDOW_HIGH_PIXEL_DENSITY);

        if (!window)
        {
            throw std::runtime_error("SDL_CreateWindow failed");
        }

        Sdl3SurfaceProvider surface_provider(window);

        auto& vulkan_ctx = VulkanContext::Get();

        vulkan_ctx.GetWindowSystemExtensions = [=](auto& extension_list)
        {
            uint32_t ext_count = 0;
            char const* const* extensions =
                SDL_Vulkan_GetInstanceExtensions(&ext_count);
            if (ext_count > 0 && extensions != nullptr)
            {
                size_t current_size = extension_list.size();
                extension_list.resize(current_size + ext_count);
                for (uint32_t i = 0; i < ext_count; ++i)
                {
                    extension_list[current_size + i] = extensions[i];
                }
            }
        };

        vulkan_ctx.Initialize("Trinity", &surface_provider);
        vulkan_ctx.RecreateSwapchain();

        Application app{GetAssetRootPath().string() + "/models/gs/sample.ply"};
        app.OnInitialize();

        bool is_running = true;
        while (is_running)
        {
            SDL_Event event;
            const Uint8* state = (const Uint8*)SDL_GetKeyboardState(nullptr);
            // Rough delta time for now
            app.ProcessInput(state, 0.016f);

            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_EVENT_QUIT)
                {
                    is_running = false;
                }
                else if (event.type == SDL_EVENT_MOUSE_MOTION)
                {
                    if (event.motion.state & SDL_BUTTON_LMASK)
                    {
                        app.ProcessMouseMotion(event.motion.xrel, event.motion.yrel);
                    }
                }
            }

            app.OnDrawFrame();
        }
        // cleanup
        app.OnCleanup();
        vulkan_ctx.Cleanup();

    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", e.what(), window);
    }

    if (window)
    {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();

    return 0;
}

#if defined(_WIN32)
int main(int argc, char *argv[])
{
    // Change current directory
    wchar_t exe_path[MAX_PATH];
    GetModuleFileNameW(nullptr, exe_path, MAX_PATH);
    fs::path exe_dir = fs::path(exe_path).parent_path();
    SetCurrentDirectoryW(exe_dir.c_str());

    fs::path asset_dir = exe_dir / "../../../assets";
    SetAssetRootPath(asset_dir);

    return RunApplication();
}
#elif defined(__linux__)
int main(int argc, char *argv[])
{
    // Change current directory
    char exe_path[PATH_MAX] = {0};
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len == -1)
    {
        std::cerr << "Failed to read /proc/self/exe" << std::endl;
    }
    exe_path[len] = '\0'; // Null-terminate
    fs::path exe_dir = fs::path(exe_path).parent_path();
    chdir(exe_dir.c_str());

    fs::path asset_dir = exe_dir / "../assets";
    SetAssetRootPath(asset_dir);

    return RunApplication();
}
#else
int main(int argc, char *argv[]) { return RunApplication(); }
#endif
