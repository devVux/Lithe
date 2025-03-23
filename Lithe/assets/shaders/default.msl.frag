#include <metal_stdlib>
using namespace metal;

struct FragmentIn {
    float4 position [[position]];
    float4 vColor;
};

fragment float4 fragment_main(FragmentIn in [[stage_in]])
{
    return in.vColor;
}
