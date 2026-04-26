#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#endif

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif

#include <filesystem>
#include <iostream>
#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>
#include "core/asset_path.h"
#include "app/application.h"

int main(int argc, char* argv[])
{
    CLI::App app{"Trinity Rendering Engine"};

    std::string title = "Trinity Sample";
    app.add_option("-t,--title", title, "Application title");

    CLI11_PARSE(app, argc, argv);

    // Create JSON
    nlohmann::json args;
    args["title"] = title;
    std::string json_str = args.dump();

#if defined(_WIN32)
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
#elif defined(__linux__)
    #include <unistd.h>
    #include <limits.h>
    // Change current directory
    char exe_path[PATH_MAX] = {0};
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len != -1)
    {
        exe_path[len] = '\0'; // Null-terminate
        std::filesystem::path exe_dir = std::filesystem::path(exe_path).parent_path();
        chdir(exe_dir.string().c_str());

        std::filesystem::path asset_dir = exe_dir / "asset";
        if (!std::filesystem::exists(asset_dir)) {
            asset_dir = exe_dir / "../asset";
        }
        tri::SetAssetRootPath(asset_dir);
    }
#endif

    return tri::Application::Run(json_str);
}
