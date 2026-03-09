#if defined(_WIN32)
#include <windows.h>
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "core/sdl2_surface_provider.h"
#include "core/swapchain.h"
#include "core/vulkan_context.h"

#include "core/asset_path.h"
#include "core/buffer_resource.h"

#include "glm/glm.hpp"

#include "triangle_app.h"

namespace fs = std::filesystem;

int runGame() {
  std::cout << "runGame started. SDL_Init..." << std::endl;
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL_Init failed" << std::endl;
    return -1;
  }

  std::cout << "Creating SDL window..." << std::endl;
  SDL_Window *window = nullptr;

  try {
    window = SDL_CreateWindow("Triangle", SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, 1280, 720,
                              SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI);

    if (!window) {
      throw std::runtime_error("SDL_CreateWindow failed");
    }

    Sdl2SurfaceProvider surfaceProvider(window);

    std::cout << "Initializing VulkanContext..." << std::endl;
    auto &vulkanCtx = VulkanContext::Get();

    vulkanCtx.GetWindowSystemExtensions = [=](auto &extensionList) {
      unsigned int extCount = 0;
      SDL_Vulkan_GetInstanceExtensions(window, &extCount, nullptr);
      if (extCount > 0) {
        size_t currentSize = extensionList.size();
        extensionList.resize(currentSize + extCount);
        SDL_Vulkan_GetInstanceExtensions(window, &extCount,
                                         extensionList.data() + currentSize);
      }
    };

    vulkanCtx.Initialize("Triangle", &surfaceProvider);
    std::cout << "Recreating swapchain..." << std::endl;
    vulkanCtx.RecreateSwapchain();

    std::cout << "Initializing TriangleApp..." << std::endl;
    TriangleApp theApp{};
    theApp.OnInitialize();

    std::cout << "Starting main event loop..." << std::endl;
    bool isRunning = true;
    uint32_t frameCount = 0;
    while (isRunning) {
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
          isRunning = false;
        }
      }

      theApp.OnDrawFrame();
      frameCount++;
      if (frameCount % 600 == 0) {
        std::cout << "Frame " << frameCount << std::endl;
      }
    }
    // cleanup
    std::cout << "Cleaning up..." << std::endl;
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
