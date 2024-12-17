#include <metal_stdlib>
using namespace metal;

fragment float4 fragment_shader(
    float4 vColor [[stage_in]]) // Input from vertex shader
{
    return vColor; // Pass the input color directly to output
}
