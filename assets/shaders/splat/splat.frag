#version 450

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec3 inConic; // conic.x, conic.y, conic.z (Actually cov_inv representation)

layout(location = 0) out vec4 outFragColor;

void main() {
    float x = inUV.x;
    float y = inUV.y;
    float opacity = inColor.a;

    // Evaluate 2D Gaussian Density
    float power = -0.5 * (inConic.x * x * x + inConic.z * y * y) - inConic.y * x * y;
    
    if (power > 0.0) discard;
    
    float alpha = min(0.99f, opacity * exp(power));
    
    if (alpha < 1.0f / 255.0f) discard;
    
    // Premultiplied Alpha
    outFragColor = vec4(inColor.rgb * alpha, alpha);
}
