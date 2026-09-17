// 3D Gaussian Splatting Shader (Slang)

struct CameraUBO {
    float4x4 view;
    float4x4 proj;
    float2 viewport;
};

[[vk::binding(0, 0)]]
ConstantBuffer<CameraUBO> camera;

struct GPUSplat {
    float4 position_opacity;
    float4 rot_scale_0;
    float4 rot_w_scale_yz;
    float4 sh_dc;
};

[[vk::binding(1, 0)]]
StructuredBuffer<GPUSplat> splats;

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
    [[vk::location(0)]] float2 uv : TEXCOORD0;
    [[vk::location(1)]] float4 color : COLOR0;
    [[vk::location(2)]] float3 conicAlpha : TEXCOORD1;
};

static const float2 quadVertices[4] = {
    float2(-1.0, -1.0),
    float2( 1.0, -1.0),
    float2(-1.0,  1.0),
    float2( 1.0,  1.0)
};

float3x3 quatToMat3(float4 q) {
    float r = q.x;
    float x = q.y;
    float y = q.z;
    float z = q.w;

    return float3x3(
        1.0 - 2.0 * (y * y + z * z), 2.0 * (x * y - r * z),       2.0 * (x * z + r * y),
        2.0 * (x * y + r * z),       1.0 - 2.0 * (x * x + z * z), 2.0 * (y * z - r * x),
        2.0 * (x * z - r * y),       2.0 * (y * z + r * x),       1.0 - 2.0 * (x * x + y * y)
    );
}

void computeCov3D(float3 scale, float mod, float4 rot, out float cov3D[6]) {
    float3x3 S = float3x3(
        mod * exp(scale.x), 0.0, 0.0,
        0.0, mod * exp(scale.y), 0.0,
        0.0, 0.0, mod * exp(scale.z)
    );

    float3x3 R = quatToMat3(rot);
    float3x3 M = mul(R, S);
    float3x3 Sigma = mul(M, transpose(M));

    cov3D[0] = Sigma[0][0];
    cov3D[1] = Sigma[0][1];
    cov3D[2] = Sigma[0][2];
    cov3D[3] = Sigma[1][1];
    cov3D[4] = Sigma[1][2];
    cov3D[5] = Sigma[2][2];
}

void computeCov2D(float3 mean, float cov3D[6], float4x4 view, float4x4 proj, float2 viewport, out float3 cov2D) {
    float4 t = mul(view, float4(mean, 1.0));

    float focal_x = proj[0][0] * viewport.x * 0.5;
    float focal_y = proj[1][1] * viewport.y * 0.5;

    float t_x = t.x;
    float t_y = t.y;
    float t_z = t.z;

    if (t_z > -0.01) {
        cov2D = float3(0.0, 0.0, 0.0);
        return;
    }

    float3x3 J = float3x3(
        focal_x / t_z, 0.0, 0.0,
        0.0, focal_y / t_z, 0.0,
        -(focal_x * t_x) / (t_z * t_z), -(focal_y * t_y) / (t_z * t_z), 0.0
    );

    float3x3 W = float3x3(
        view[0].xyz,
        view[1].xyz,
        view[2].xyz
    );

    float3x3 T = mul(J, W);

    float3x3 Vrk = float3x3(
        cov3D[0], cov3D[1], cov3D[2],
        cov3D[1], cov3D[3], cov3D[4],
        cov3D[2], cov3D[4], cov3D[5]
    );

    float3x3 cov = mul(mul(T, Vrk), transpose(T));

    cov[0][0] += 0.3;
    cov[1][1] += 0.3;

    cov2D = float3(cov[0][0], cov[0][1], cov[1][1]);
}

static const float SH_C0 = 0.28209479177387814;

[shader("vertex")]
VSOutput vsMain(uint vertexId : SV_VertexID, uint instanceId : SV_InstanceID) {
    VSOutput output;

    uint splatIdx = indices[instanceId];
    GPUSplat splat = splats[splatIdx];

    float3 mean = splat.position_opacity.xyz;
    float opacity = 1.0 / (1.0 + exp(-splat.position_opacity.w)); // sigmoid

    float4 rot = float4(splat.rot_w_scale_yz.x, splat.rot_scale_0.xyz); // w, x, y, z
    float3 scale = float3(splat.rot_scale_0.w, splat.rot_w_scale_yz.y, splat.rot_w_scale_yz.z);

    // Compute SH to Color (Basic Degree 0)
    float3 color = SH_C0 * splat.sh_dc.xyz + 0.5;
    color = max(float3(0.0, 0.0, 0.0), min(float3(1.0, 1.0, 1.0), color));

    // Project Covariance to 2D
    float cov3D[6];
    computeCov3D(scale, 1.0, rot, cov3D);

    float4x4 model = transforms[pc.matrixIndex];
    float3 worldMean = mul(model, float4(mean, 1.0)).xyz;

    float3 cov2D;
    computeCov2D(worldMean, cov3D, camera.view, camera.proj, camera.viewport, cov2D);

    // Compute Conic
    float det = (cov2D.x * cov2D.z - cov2D.y * cov2D.y);
    if (det == 0.0) det = 0.0000001; // Avoid div by zero
    float det_inv = 1.0 / det;
    float3 conic = float3(cov2D.z * det_inv, -cov2D.y * det_inv, cov2D.x * det_inv);

    // Compute Screen Space Bound Radius
    float mid = 0.5 * (cov2D.x + cov2D.z);
    float lambda1 = mid + sqrt(max(0.1, mid * mid - det));
    float lambda2 = mid - sqrt(max(0.1, mid * mid - det));
    float my_radius = ceil(3.0 * sqrt(max(lambda1, lambda2)));

    // Set outputs
    float4 p_hom = mul(camera.proj, mul(camera.view, float4(worldMean, 1.0)));
    float2 offset = quadVertices[vertexId] * my_radius / camera.viewport * 2.0;

    output.position = float4(p_hom.xy + offset * p_hom.w, p_hom.z, p_hom.w);
    output.uv = quadVertices[vertexId] * my_radius;
    output.color = float4(color, opacity);
    output.conicAlpha = conic;

    return output;
}

[shader("fragment")]
float4 fsMain(VSOutput input) : SV_Target {
    float x = input.uv.x;
    float y = input.uv.y;
    float opacity = input.color.a;

    // Evaluate 2D Gaussian Density
    float power = -0.5 * (input.conicAlpha.x * x * x + input.conicAlpha.z * y * y) - input.conicAlpha.y * x * y;

    if (power > 0.0) discard;

    float alpha = min(0.99, opacity * exp(power));

    if (alpha < 1.0 / 255.0) discard;

    // Premultiplied Alpha
    return float4(input.color.rgb * alpha, alpha);
}
