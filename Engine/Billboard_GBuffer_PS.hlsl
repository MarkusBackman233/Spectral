#include "common.hlsli"
Texture2D albedoMap : register(t0);

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

SamplerState samplerState : register(s0);
SamplerComparisonState cmpSampler : register(s1);
SamplerState cubeSampler : register(s2);
SamplerState clampSampler : register(s3);
#define PI 3.14159265


cbuffer ViewConstantBuffer : register(b0)
{
    float3 cameraRight;
    float3 cameraUp;
    float3 cameraForward;
    
    float3 unused;
};

uint4 main(PS_INPUT input) : SV_TARGET
{
    float4 sample = albedoMap.Sample(samplerState, input.uv);
    clip(sample.a - 0.25);

    // UV -> [-1,1]
    float2 p = input.uv * 2.0f - 1.0f;
    p.y = -p.y;

    float r2 = dot(p, p);

    // Clip outside sphere
    clip(1.0f - r2);

    // Hemisphere Z
    float z = sqrt(1.0f - r2);

    // Billboard-local normal
    float3 localNormal = float3(p.x, p.y, z);

    // Convert to world space
    float3 worldNormal =
        localNormal.x * -cameraRight +
        localNormal.y * cameraUp +
        localNormal.z * -cameraForward;

    worldNormal = normalize(worldNormal);

    return CreateGBuffer(
        worldNormal,
        sample.rgb,
        0.97,
        1.0,
        1.0,
        1
    );
}