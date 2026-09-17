#version 450

// Camera uniforms
layout(binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
    vec2 viewport;
} camera;

// Compact GPU Point data (32 bytes per point)
struct GPUPoint {
    vec4 position; // xyz: position, w: reserved/scale
    vec4 color;    // rgb: color, a: opacity
};

// SSBO containing the point cloud data
layout(std430, binding = 1) readonly buffer PointBuffer {
    GPUPoint points[];
};

// SSBO containing the sorted indices
layout(std430, binding = 2) readonly buffer IndexBuffer {
    uint indices[];
};

// Transform Buffer (Batch transfer from Scene)
layout(std430, binding = 3) readonly buffer TransformBuffer {
    mat4 transforms[];
};

// Push Constants
layout(push_constant) uniform PushConstants {
    uint matrixIndex;
    float pointSize;
} pc;

// Output to Fragment Shader
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outUV;

// Quad vertices for two triangles forming a quad billboard (6 vertices)
const vec2 quadVertices[6] = vec2[](
    vec2(-0.5, -0.5),
    vec2( 0.5, -0.5),
    vec2(-0.5,  0.5),
    vec2(-0.5,  0.5),
    vec2( 0.5, -0.5),
    vec2( 0.5,  0.5)
);

void main() {
    uint ptIdx = indices[gl_InstanceIndex];
    GPUPoint pt = points[ptIdx];

    mat4 model = transforms[pc.matrixIndex];
    vec3 worldPos = (model * vec4(pt.position.xyz, 1.0)).xyz;

    // View-space billboard: camera-facing quad polygon
    vec4 viewPos = camera.view * vec4(worldPos, 1.0);

    float size = pc.pointSize;
    vec2 offset = quadVertices[gl_VertexIndex] * size;
    viewPos.xy += offset;

    gl_Position = camera.proj * viewPos;
    outColor = pt.color;
    outUV = quadVertices[gl_VertexIndex] + 0.5;
}
