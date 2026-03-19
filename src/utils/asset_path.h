#pragma once
#include <filesystem>

// Sets the asset root path
void SetAssetRootPath(const std::filesystem::path& path);

// Gets the current asset root path
std::filesystem::path GetAssetRootPath();

// Asset type
enum class AssetType
{
	Shader = 0,
	Texture,
	Model,
	AssetTypeMax,
};
// Gets the file path for a specific asset type
std::filesystem::path GetAssetPath(AssetType type, const std::filesystem::path& fileName);
