#pragma once
#include "stream/splat_types.h"
#include <string>
#include <vector>

namespace trinity::stream {


class PlyLoader
{
public:
    // Loads a standard 3DGS .ply file
    // Returns true on success, false on failure
    static bool LoadPly(const std::string& filepath,
        std::vector<trinity::stream::FullSplat>& out_splats);
};

} // namespace trinity::stream
