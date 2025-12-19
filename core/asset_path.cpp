#include "core/asset_path.h"
#include <array>
#include <string_view>

namespace {
    std::filesystem::path asset_root__ = "assets";
    std::string_view ToSubDirectoryName(AssetType type) {
        constexpr std::array<std::string_view, int(AssetType::AssetTypeCount)> asset_dir_names = {
            "shaders",
            "textures",
            "models",
        };
        return asset_dir_names[static_cast<int>(type)];
    }
}

void SetAssetRootPath(const std::filesystem::path& path) {
    auto full_path = std::filesystem::absolute(path);
    asset_root__ = std::filesystem::canonical(full_path);
}

std::filesystem::path GetAssetRootPath() {
    return asset_root__;
}

std::filesystem::path GetAssetPath(AssetType type, const std::filesystem::path& file_name) {
    return GetAssetRootPath() / ToSubDirectoryName(type) / file_name;
}