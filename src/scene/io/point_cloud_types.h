#pragma once
#include <glm/glm.hpp>
#include <cstdint>

namespace tri
{
    struct PointCloudVertex
    {
        glm::vec3 position;
        glm::vec3 color;
    };

    struct GpuPoint
    {
        glm::vec4 position;
        glm::vec4 color;
    };

    struct PointSortEntry
    {
        uint32_t index;
        float depth;
    };
} // namespace tri

