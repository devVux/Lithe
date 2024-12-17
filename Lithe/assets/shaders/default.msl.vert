#include <metal_stdlib>
using namespace metal;

struct VertexIn {
    float2 position [[attribute(0)]];
    float4 color [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]];
    float4 vColor;
};

vertex VertexOut vertex_shader(VertexIn in [[stage_in]]) {
    VertexOut out;
    out.position = float4(in.position, 0.0, 1.0); // Transform position to vec4
    out.vColor = in.color; // Pass color through to fragment shader
    return out;
}
