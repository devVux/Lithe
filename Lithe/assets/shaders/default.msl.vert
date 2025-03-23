#include <metal_stdlib>
using namespace metal;

struct CameraBuffer {
    float4x4 uViewProjection;
};

struct EntityBuffer {
    float4x4 uTransforms[100];
    float4 uColors[100];
};

struct VertexIn {
    float3 position [[attribute(0)]];
};

struct VertexOut {
    float4 position [[position]];
    float4 vColor;
};

vertex VertexOut vertex_main(
    VertexIn in [[stage_in]],
    constant CameraBuffer& camera [[buffer(0)]],
    constant EntityBuffer& entity [[buffer(1)]],
    uint instanceID [[instance_id]])
{
    VertexOut out;
    out.vColor = entity.uColors[instanceID];
    float4x4 model = entity.uTransforms[instanceID];
    out.position = camera.uViewProjection * model * float4(in.position, 1.0);
    return out;
}
