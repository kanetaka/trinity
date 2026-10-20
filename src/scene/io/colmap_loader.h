#pragma once
#include "scene/io/point_cloud_types.h"
#include <string>
#include <vector>

namespace tri
{
    class ColmapLoader
    {
    public:
        // Returns true if the path points to a COLMAP sparse model
        // (a points3D.bin/points3D.txt file or a directory containing one)
        static bool IsColmapModel(const std::string& path);

        // Loads the 3D points of a COLMAP sparse model (binary or text format)
        // Returns true on success, false on failure
        static bool LoadPointCloud(const std::string& path,
            std::vector<PointCloudVertex>& out_vertices);
    };
} // namespace tri
