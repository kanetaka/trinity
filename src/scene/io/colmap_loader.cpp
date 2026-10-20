#include "scene/io/colmap_loader.h"
#include "core/logger.h"
#include <exception>
#include <filesystem>
#include <iostream>

#include <colmap/scene/reconstruction.h>

using namespace tri;

namespace
{
    std::filesystem::path GetModelDirectory(const std::string& path)
    {
        std::filesystem::path model_path(path);
        if (std::filesystem::is_directory(model_path))
        {
            return model_path;
        }
        return model_path.parent_path();
    }
} // namespace

bool ColmapLoader::IsColmapModel(const std::string& path)
{
    std::filesystem::path model_path(path);
    if (!std::filesystem::is_directory(model_path) && model_path.stem() != "points3D")
    {
        return false;
    }

    std::filesystem::path model_dir = GetModelDirectory(path);
    return std::filesystem::exists(model_dir / "points3D.bin")
        || std::filesystem::exists(model_dir / "points3D.txt");
}

bool ColmapLoader::LoadPointCloud(const std::string& path, std::vector<PointCloudVertex>& out_vertices)
{
    std::filesystem::path model_dir = GetModelDirectory(path);

    colmap::Reconstruction reconstruction;
    try
    {
        // Detects binary or text format automatically
        reconstruction.Read(model_dir.string());
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to read COLMAP model: " << model_dir.string() << " (" << e.what() << ")" << std::endl;
        return false;
    }

    out_vertices.clear();
    out_vertices.reserve(reconstruction.NumPoints3D());
    for (const auto& [point3d_id, point3d] : reconstruction.Points3D())
    {
        PointCloudVertex v;
        v.position = glm::vec3(
            static_cast<float>(point3d.xyz.x()),
            static_cast<float>(point3d.xyz.y()),
            static_cast<float>(point3d.xyz.z()));
        v.color = glm::vec3(
            point3d.color.x() / 255.0f,
            point3d.color.y() / 255.0f,
            point3d.color.z() / 255.0f);
        out_vertices.push_back(v);
    }

    TRI_LOG_INFO("ColmapLoader", "Loaded COLMAP model: " + model_dir.string()
        + " (cameras: " + std::to_string(reconstruction.NumCameras())
        + ", images: " + std::to_string(reconstruction.NumImages())
        + ", points: " + std::to_string(out_vertices.size()) + ")");
    return true;
}
