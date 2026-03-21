#pragma once
#include <cstdint>
#include <vector>
#include <glm/glm.hpp>

namespace trinity::stream::gs {

struct Splat {
    glm::vec3 position;
    float scale_x;
    glm::vec3 normal;
    float scale_y;
    glm::vec3 sh_dc;
    float scale_z;
    float sh_rest[45];
    float opacity;
    glm::vec4 rotation;
};

struct GpuSplat {
    glm::vec4 position_opacity; // xyz = position, w = opacity
    glm::vec4 rot_scale_0;      // xyz = rotation (xyz), w = scale_x
    glm::vec4 rot_w_scale_yz;   // x = rotation w, y = scale_y, z = scale_z
    glm::vec4 sh_dc;            // xyz = sh_dc (color), w = padding
};

struct FullSplat {
    glm::vec3 position;
    glm::vec3 normal;
    float sh_dc[3];
    float sh_rest[45];
    float opacity;
    glm::vec3 scale;
    glm::vec4 rot;
};

struct SplatSortEntry {
    uint32_t index;
    float depth;
};

} // namespace trinity::stream::gs
