// PointCloud Billboard Quad Shader (Slang)

struct CameraUBO {
    float4x4 view;
    float4x4 proj;
    float2 viewport;
};

[[vk::binding(0, 0)]]
ConstantBuffer<CameraUBO> camera;

struct GPUPoint {
    float4 position;
    float4 color;
};

[[vk::binding(1, 0)]]
StructuredBuffer<GPUPoint> points;

[[vk::binding(2, 0)]]
StructuredBuffer<uint> indices;

[[vk::binding(3, 0)]]
StructuredBuffer<float4x4> transforms;

struct PushConstants {
    uint matrixIndex;
    float pointSize;
};

[[vk::push_constant]]
ConstantBuffer<PushConstants> pc;

struct VSOutput {
    float4 position : SV_Position;
    [[vk::location(0)]] float4 color : COLOR0;
    [[vk::location(1)]] float2 uv : TEXCOORD0;
};

static const float2 quadVertices[6] = {
    float2(-0.5, -0.5),
    float2( 0.5, -0.5),
    float2(-0.5,  0.5),
    float2(-0.5,  0.5),
    float2( 0.5, -0.5),
    float2( 0.5,  0.5)
};

[shader("vertex")]
VSOutput vsMain(uint vertexId : SV_VertexID, uint instanceId : SV_InstanceID) {
    VSOutput output;

    uint ptIdx = indices[instanceId];
    GPUPoint pt = points[ptIdx];

    float4x4 model = transforms[pc.matrixIndex];
    float3 worldPos = mul(model, float4(pt.position.xyz, 1.0)).xyz;

    // View-space billboard: camera-facing quad polygon
    float4 viewPos = mul(camera.view, float4(worldPos, 1.0));

    float size = pc.pointSize;
    float2 offset = quadVertices[vertexId] * size;
    viewPos.xy += offset;

    output.position = mul(camera.proj, viewPos);
    output.color = pt.color;
    output.uv = quadVertices[vertexId] + 0.5;

    return output;
}

[shader("fragment")]
float4 fsMain(VSOutput input) : SV_Target {
    return input.color;
}
