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
    float3 CameraPos;
    float unused4;
};


float3 SampleColor(float2 inputPos, float normaly, float uvScale)
{
    float2 tUV = inputPos * uvScale;
        
    float3 t0 = terrainTextures[0].Sample(samplerState, tUV).rgb;
    float3 t1 = terrainTextures[1].Sample(samplerState, tUV).rgb;
    float3 t2 = terrainTextures[2].Sample(samplerState, tUV).rgb;
    float3 t3 = terrainTextures[3].Sample(samplerState, tUV).rgb;
        
        

    
    float slope = 1.0 - saturate(normaly);
    float rockFactor = smoothstep(0.0, 0.4, slope);
    float3 base = lerp(t0, t1, rockFactor);

    float t2Factor = 1.0 - smoothstep(0.0, 0.05, slope);
    base = lerp(base, t2, t2Factor);
    return base;
}

uint4 main(VS_OUTPUT input) : SV_Target
{
    
    float d = saturate(length((input.worldPos - CameraPos)) / 100.0f);
    
    
    float3 farAway = SampleColor(input.worldPos.xz, input.normal.y, 0.0025);
    float2 uv = input.worldPos.xz / WorldSize;
    float3 wc = worldColor.Sample(samplerState, uv).rgb;
    wc = pow(wc, 1 / 0.5);
    farAway = lerp(farAway, wc, 0.5);
    
    
    
    float3 base = lerp(SampleColor(input.worldPos.xz, input.normal.y, 0.1), farAway, d);
    

    
    return CreateGBuffer(input.normal, base, 1.0f, 0.0, 1.0, 0);
}