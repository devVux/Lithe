#include <metal_stdlib>
using namespace metal;

struct CameraBuffer {
    float4x4 uViewProjection;
};

struct VertexIn {
    float2 position [[attribute(0)]];
    float4 color [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]];
    float4 vColor;
};

vertex VertexOut vertex_shader(VertexIn in [[stage_in]], constant CameraBuffer& camera [[buffer(0)]]) {
    VertexOut out;
    out.vColor = in.color;
    out.position = camera.uViewProjection * float4(in.position, 0.0, 1.0);
    return out;
}
