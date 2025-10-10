#include "common.hlsli"

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    bool isFrontFace : SV_IsFrontFace;
};

SamplerState samplerState : register(s0);
SamplerComparisonState cmpSampler : register(s1);
SamplerState cubeSampler : register(s2);
SamplerState clampSampler : register(s3);
#define PI 3.14159265



uint4 main(PS_INPUT input) : SV_TARGET
{
    
    if (!input.isFrontFace)
    {
        input.normal.xyz = -input.normal.xyz;
    }
    
    return CreateGBuffer(input.normal.xyz, input.color.rgb, 0.6, 1.0, input.color.a, 1);
}