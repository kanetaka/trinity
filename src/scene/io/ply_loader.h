#pragma once
#include "scene/io/splat_types.h"
#include "scene/io/point_cloud_types.h"
#include <string>
#include <vector>

namespace tri
{
    class PlyLoader
    {
    public:
        // Loads a standard 3DGS .ply file
        // Returns true on success, false on failure
        static bool LoadPly(const std::string& filepath,
            std::vector<FullSplat>& out_splats);

        // Loads a point cloud .ply file (ASCII or binary little-endian)
        // Returns true on success, false on failure
        static bool LoadPointCloud(const std::string& filepath,
            std::vector<PointCloudVertex>& out_vertices);
    };
} // namespace tri

