#include "core/asset_path.h"
#include <array>
#include <string_view>

using namespace tri;

namespace
{
    std::filesystem::path asset_root__ = "asset/";

    std::string_view ToSubDirectoryName(AssetType type)
    {
        constexpr std::array<std::string_view, int(AssetType::AssetTypeMax)> kAssetDirs = {
            "shader", "texture", "model",
        };
        return kAssetDirs[int(type)];
    }
}

void tri::SetAssetRootPath(const std::filesystem::path& path)
{
    asset_root__ = std::filesystem::absolute(path);
}

std::filesystem::path tri::GetAssetRootPath()
{
    return asset_root__;
}

std::filesystem::path tri::GetAssetPath(AssetType type, const std::filesystem::path& fileName)
{
    return GetAssetRootPath() / ToSubDirectoryName(type) / fileName;
}
