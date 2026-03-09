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

#include "core/sdl3_surface_provider.h"
#include "core/swapchain.h"
#include "core/vulkan_context.h"

#include "core/asset_path.h"
#include "core/buffer_resource.h"

#include "glm/glm.hpp"

#include "gs_app.h"

namespace fs = std::filesystem;

int runGame() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "SDL_Init failed" << std::endl;
    return -1;
  }

  SDL_Window *window = nullptr;

  try {
    window =
        SDL_CreateWindow("Triangle", 1280, 720,
                         SDL_WINDOW_VULKAN | SDL_WINDOW_HIGH_PIXEL_DENSITY);

    if (!window) {
      throw std::runtime_error("SDL_CreateWindow failed");
    }

    Sdl3SurfaceProvider surfaceProvider(window);

    auto &vulkanCtx = VulkanContext::Get();

    vulkanCtx.GetWindowSystemExtensions = [=](auto &extensionList) {
      uint32_t extCount = 0;
      char const *const *extensions =
          SDL_Vulkan_GetInstanceExtensions(&extCount);
      if (extCount > 0 && extensions != nullptr) {
        size_t currentSize = extensionList.size();
        extensionList.resize(currentSize + extCount);
        for (uint32_t i = 0; i < extCount; ++i) {
          extensionList[currentSize + i] = extensions[i];
        }
      }
    };

    vulkanCtx.Initialize("Triangle", &surfaceProvider);
    vulkanCtx.RecreateSwapchain();

    GsApp theApp{GetAssetRootPath().string() + "/splat.ply"};
    theApp.OnInitialize();

    bool isRunning = true;
    while (isRunning) {
      SDL_Event event;
      const Uint8 *state = (const Uint8 *)SDL_GetKeyboardState(nullptr);
      // Rough delta time for now
      theApp.ProcessInput(state, 0.016f);

      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
          isRunning = false;
        } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
          if (event.motion.state & SDL_BUTTON_LMASK) {
            theApp.ProcessMouseMotion(event.motion.xrel, event.motion.yrel);
          }
        }
      }

      theApp.OnDrawFrame();
    }
    // cleanup
    theApp.OnCleanup();
    vulkanCtx.Cleanup();

  } catch (const std::exception &e) {
    std::cerr << "Fatal Error: " << e.what() << std::endl;
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", e.what(),
                             window);
  }

  if (window)
    SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}

#if defined(_WIN32)
int main(int argc, char *argv[]) {
  // カレントディレクトリを変更
  wchar_t exePath[MAX_PATH];
  GetModuleFileNameW(nullptr, exePath, MAX_PATH);
  fs::path exeDir = fs::path(exePath).parent_path();
  SetCurrentDirectoryW(exeDir.c_str());

  fs::path assetDir = exeDir / "../../../assets";
  SetAssetRootPath(assetDir);

  return runGame();
}
#elif defined(__linux__)
int main(int argc, char *argv[]) {
  // カレントディレクトリを変更
  char exePath[PATH_MAX] = {0};
  ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
  if (len == -1) {
    std::cerr << "Failed to read /proc/self/exe" << std::endl;
  }
  exePath[len] = '\0'; // NULLで終端
  fs::path exeDir = fs::path(exePath).parent_path();
  chdir(exeDir.c_str());

  fs::path assetDir = exeDir / "../assets";
  SetAssetRootPath(assetDir);

  return runGame();
}
#else
int main(int argc, char *argv[]) { return runGame(); }
#endif
