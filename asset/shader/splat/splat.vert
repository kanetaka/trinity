#version 450

// Camera uniforms
layout(binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
    vec2 viewport; // width, height
} camera;

// Structured Buffer for Splat Data
struct GPUSplat {
    vec4 position_opacity;
    vec4 rot_scale_0;
    vec4 rot_w_scale_yz;
    vec4 sh_dc;
};

// SSBO containing the actual splats
layout(std430, binding = 1) readonly buffer SplatBuffer {
    GPUSplat splats[];
};

// SSBO containing the sorted indices
layout(std430, binding = 2) readonly buffer IndexBuffer {
    uint indices[];
};

// Output to Fragment Shader
layout(location = 0) out vec2 outUV;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec3 outConicAlpha; // conic.x, conic.y, conic.z (opacity)

// Transform Buffer (Batch transfer from ECS)
layout(std430, binding = 3) readonly buffer TransformBuffer {
    mat4 transforms[];
};

// Push Constants
layout(push_constant) uniform PushConstants {
    uint matrixIndex;
} pc;

// Quad vertices
const vec2 quadVertices[4] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0,  1.0)
);

// Math utilities
mat3 quatToMat3(vec4 q) {
    float r = q.x; // Re-mapped to w theoretically
    float x = q.y;
    float y = q.z;
    float z = q.w;

    // Actually, based on PLY structure, usually it's w, x, y, z in rot_0, rot_1, rot_2, rot_3
    // We packed it as: rot_scale_0.xyz = x,y,z; rot_w_scale_yz.x = w
    // Let's assume standard normalized quaternion

    return mat3(
        1.0 - 2.0 * (y * y + z * z), 2.0 * (x * y - r * z),       2.0 * (x * z + r * y),
        2.0 * (x * y + r * z),       1.0 - 2.0 * (x * x + z * z), 2.0 * (y * z - r * x),
        2.0 * (x * z - r * y),       2.0 * (y * z + r * x),       1.0 - 2.0 * (x * x + y * y)
    );
}

void computeCov3D(vec3 scale, float mod, vec4 rot, out float cov3D[6]) {
    mat3 S = mat3(
        mod * exp(scale.x), 0.0, 0.0,
        0.0, mod * exp(scale.y), 0.0,
        0.0, 0.0, mod * exp(scale.z)
    );

    mat3 R = quatToMat3(rot);
    mat3 M = R * S;
    mat3 Sigma = M * transpose(M);

    cov3D[0] = Sigma[0][0];
    cov3D[1] = Sigma[0][1];
    cov3D[2] = Sigma[0][2];
    cov3D[3] = Sigma[1][1];
    cov3D[4] = Sigma[1][2];
    cov3D[5] = Sigma[2][2];
}

void computeCov2D(vec3 mean, float cov3D[6], mat4 view, mat4 proj, vec2 viewport, out vec3 cov2D) {
    vec4 t = view * vec4(mean, 1.0);

    // Focal lengths
    float focal_x = proj[0][0] * viewport.x * 0.5;
    float focal_y = proj[1][1] * viewport.y * 0.5;

    float t_x = t.x;
    float t_y = t.y;
    float t_z = t.z;
    
    // Bounds check to avoid singularity at camera center
    if (t_z > -0.01) {
        cov2D = vec3(0.0);
        return;
    }

    mat3 J = mat3(
        focal_x / t_z, 0.0, 0.0,
        0.0, focal_y / t_z, 0.0,
        -(focal_x * t_x) / (t_z * t_z), -(focal_y * t_y) / (t_z * t_z), 0.0
    );

    mat3 W = mat3(view);

    mat3 T = J * W;
    
    mat3 Vrk = mat3(
        cov3D[0], cov3D[1], cov3D[2],
        cov3D[1], cov3D[3], cov3D[4],
        cov3D[2], cov3D[4], cov3D[5]
    );

    mat3 cov = T * Vrk * transpose(T);
    
    // Low pass filter
    cov[0][0] += 0.3;
    cov[1][1] += 0.3;

    cov2D = vec3(cov[0][0], cov[0][1], cov[1][1]);
}

// Spherical harmonics base color
const float SH_C0 = 0.28209479177387814;

void main() {
    uint splatIdx = indices[gl_InstanceIndex];
    GPUSplat splat = splats[splatIdx];
    
    vec3 mean = splat.position_opacity.xyz;
    float opacity = 1.0 / (1.0 + exp(-splat.position_opacity.w)); // sigmoid
    
    vec4 rot = vec4(splat.rot_w_scale_yz.x, splat.rot_scale_0.xyz); // w, x, y, z
    vec3 scale = vec3(splat.rot_scale_0.w, splat.rot_w_scale_yz.y, splat.rot_w_scale_yz.z);
    
    // Compute SH to Color (Basic Degree 0)
    vec3 color = SH_C0 * splat.sh_dc.xyz + 0.5;
    color = max(vec3(0.0), min(vec3(1.0), color));
    
    // Project Covariance to 2D
    float cov3D[6];
    computeCov3D(scale, 1.0, rot, cov3D);
    
    mat4 model = transforms[pc.matrixIndex];
    vec3 worldMean = (model * vec4(mean, 1.0)).xyz;

    vec3 cov2D;
    computeCov2D(worldMean, cov3D, camera.view, camera.proj, camera.viewport, cov2D);
    
    // Compute Conic
    float det = (cov2D.x * cov2D.z - cov2D.y * cov2D.y);
    if (det == 0.0) det = 0.0000001; // Avoid div by zero
    float det_inv = 1.0 / det;
    vec3 conic = vec3(cov2D.z * det_inv, -cov2D.y * det_inv, cov2D.x * det_inv);
    
    // Compute Screen Space Bound Radius
    float mid = 0.5 * (cov2D.x + cov2D.z);
    float lambda1 = mid + sqrt(max(0.1, mid * mid - det));
    float lambda2 = mid - sqrt(max(0.1, mid * mid - det));
    float my_radius = ceil(3.0 * sqrt(max(lambda1, lambda2)));
 
    // Set outputs
    vec4 p_hom = camera.proj * camera.view * vec4(worldMean, 1.0);
    // Add offset based on quad vertex
    vec2 offset = quadVertices[gl_VertexIndex] * my_radius / camera.viewport * 2.0;
    
    gl_Position = vec4(p_hom.xy + offset * p_hom.w, p_hom.z, p_hom.w);
    outUV = quadVertices[gl_VertexIndex] * my_radius;
    outColor = vec4(color, opacity);
    outConicAlpha = vec3(conic.x, conic.y, conic.z);
}
