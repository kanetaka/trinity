#include "core/asset_path.h"
#include <array>
#include <string_view>

namespace {
    std::filesystem::path asset_root__ = "assets/";

    std::string_view ToSubDirectoryName(AssetType type)
    {
        constexpr std::array<std::string_view, int(AssetType::AssetTypeMax)> kAssetDirs = {
            "shaders", "textures", "models",
        };
        return kAssetDirs[int(type)];
    }
}

void SetAssetRootPath(const std::filesystem::path& path)
{
    auto fullPath = std::filesystem::absolute(path);
    asset_root__ = std::filesystem::canonical(fullPath);
}

std::filesystem::path GetAssetRootPath()
{
    return asset_root__;
}

std::filesystem::path GetAssetPath(AssetType type, const std::filesystem::path& fileName)
{
    return GetAssetRootPath() / ToSubDirectoryName(type) / fileName;
}
