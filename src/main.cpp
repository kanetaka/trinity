#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#endif

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif

#include <filesystem>
#include <iostream>
#include "core/asset_path.h"
#include "app/application.h"

#if defined(_WIN32)
int main(int argc, char *argv[])
{
    // Change current directory
    wchar_t exe_path[MAX_PATH];
    GetModuleFileNameW(nullptr, exe_path, MAX_PATH);
    std::filesystem::path exe_dir = std::filesystem::path(exe_path).parent_path();
    SetCurrentDirectoryW(exe_dir.c_str());

    std::filesystem::path asset_dir = exe_dir / "asset";
    if (!std::filesystem::exists(asset_dir)) {
        asset_dir = exe_dir / "../asset";
    }
    if (!std::filesystem::exists(asset_dir)) {
        asset_dir = exe_dir / "../../asset";
    }
    if (!std::filesystem::exists(asset_dir)) {
        asset_dir = exe_dir / "../../../asset";
    }
    tri::SetAssetRootPath(asset_dir);

    return tri::Application::Run(argc, argv);
}
#elif defined(__linux__)
#include <unistd.h>
#include <limits.h>
int main(int argc, char *argv[])
{
    // Change current directory
    char exe_path[PATH_MAX] = {0};
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len == -1)
    {
        std::cerr << "Failed to read /proc/self/exe" << std::endl;
        return -1;
    }
    exe_path[len] = '\0'; // Null-terminate
    std::filesystem::path exe_dir = std::filesystem::path(exe_path).parent_path();
    chdir(exe_dir.string().c_str());

    std::filesystem::path asset_dir = exe_dir / "asset";
    if (!std::filesystem::exists(asset_dir)) {
        asset_dir = exe_dir / "../asset";
    }
    tri::SetAssetRootPath(asset_dir);

    return tri::Application::Run(argc, argv);
}
#else
int main(int argc, char *argv[]) 
{ 
    return tri::Application::Run(argc, argv); 
}
#endif
