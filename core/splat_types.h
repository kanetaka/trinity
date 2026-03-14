#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>


namespace gs {

// Structured data for a single 3D Gaussian Splat
// This layout matches exactly what will be sent to the GPU Storage Buffer
struct Splat {
    glm::vec3 position;
    float scale_x;

    glm::vec3 normal; // Usually not used for rendering 3DGS, but part of PLY
    float scale_y;

    // SH Degree 0 Color
    glm::vec3 sh_dc;
    float scale_z;

    // SH Higher Degrees (1-3) - 45 floats
    // To keep it simple, we use an array. For 0 degree, this will be zeroes
    float sh_rest[45];

    float opacity;
    glm::vec4 rotation; // Quaternion (w, x, y, z) typically, but check PLY format
                                            // (x, y, z, w?)
};

// Simplified splat data to upload to GPU
// We pre-calculate or restructure data to be optimal for GPU access
struct GPUSplat {
    glm::vec4 position_opacity; // xyz = position, w = opacity
    glm::vec4 rot_scale_0;    // xyz = rotation (xyz), w = scale_x (or we can just
                                                        // use quat)
    glm::vec4 rot_w_scale_yz; // x = rotation w, y = scale_y, z = scale_z
    glm::vec4 sh_dc;          // xyz = sh_dc (color), w = padding

    // For full SH, we'd need more data.
    // Let's stick to SH degree 0 (base color) for the first iteration to get
    // it working, then expand to degree 3 if needed.
};

// Full features splat for CPU side parsing
struct FullSplat {
    glm::vec3 position;
    glm::vec3 normal;
    float sh_dc[3];
    float sh_rest[45];
    float opacity;
    glm::vec3 scale;
    glm::vec4 rot; // x,y,z,w
};

// Simple structure for sorting
struct SplatSortEntry {
    uint32_t index;
    float depth;
};

} // namespace gs
