#pragma once
#include "core/splat_types.h"
#include <string>
#include <vector>

namespace trinity::stream {





namespace gs
{
    class PlyLoader
    {
    public:
        // Loads a standard 3DGS .ply file
        // Returns true on success, false on failure
        static bool LoadPly(const std::string& filepath,
            std::vector<trinity::core::gs::FullSplat>& out_splats);
    };
} // namespace gs


} // namespace trinity::stream
