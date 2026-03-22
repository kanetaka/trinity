#pragma once
#include "stream/splat_types.h"
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
    };
} // namespace tri
