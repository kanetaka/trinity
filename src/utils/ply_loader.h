#pragma once
#include "core/renderer/splat_types.h"
#include <string>
#include <vector>


namespace gs
{
	class PlyLoader
	{
	public:
		// Loads a standard 3DGS .ply file
		// Returns true on success, false on failure
		static bool LoadPly(const std::string& filepath,
			std::vector<FullSplat>& out_splats);
	};
} // namespace gs
