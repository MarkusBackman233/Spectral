#include "common.hlsli"

Texture2D worldColor : register(t0);
Texture2D terrainTextures[8]; // albedo
Texture2D normalTextures[8]; // normals
SamplerState samplerState : register(s0);
SamplerComparisonState cmpSampler : register(s1);
SamplerState cubeSampler : register(s2);
SamplerState clampSampler : register(s3);
SamplerState pointSampler : register(s4);

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float3 binormal : TEXCOORD3;
};


cbuffer PixelConstantBuffer : register(b1)
{
    float3 CB_mouseRaycastHit;
    float CB_brushSize;
    unsigned int CB_materialData;
    float WorldSize;
    float unused2;
    float unused3;
};

uint4 main(VS_OUTPUT input) : SV_Target
{
    
    float2 tUV = input.worldPos.xz * 0.0025;
    
    float3 t0 = terrainTextures[0].Sample(samplerState, tUV).rgb;
    float3 t1 = terrainTextures[1].Sample(samplerState, tUV).rgb;
    float3 t2 = terrainTextures[2].Sample(samplerState, tUV).rgb;
    float3 t3 = terrainTextures[3].Sample(samplerState, tUV).rgb;
    
    
    float2 uv = input.worldPos.xz / WorldSize;
    
    float3 wc = worldColor.Sample(samplerState, uv).rgb;
    input.worldPos.y += length(wc) * 10.0f + terrainTextures[3].Sample(samplerState, input.worldPos.xz*0.01).r*0.1;
    
    float3 dx = ddx(input.worldPos);
    float3 dy = ddy(input.worldPos);

    float3 normal = -normalize(cross(dy, dx));

    float slope = 1.0 - saturate(normal.y);

    float rockFactor = smoothstep(0.0, 0.4, slope);


    

    
    float3 base = lerp(t0, t1, rockFactor);

    wc = pow(wc, 1 / 0.5);
    
    float t2Factor = 1.0 - smoothstep(0.0, 0.05, slope);
    base = lerp(base, t2, t2Factor);
    base = lerp(base, wc, 0.5);

    return CreateGBuffer(normal, base, 1.0f, 0.0, 1.0, 0);
}