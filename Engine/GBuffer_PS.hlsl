#include "common.hlsli"

Texture2D albedoMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D roughnessMap : register(t2);
Texture2D metallicMap : register(t3);
Texture2D aoMap : register(t4);

SamplerState samplerState : register(s0);
SamplerComparisonState cmpSampler : register(s1);
SamplerState cubeSampler : register(s2);
SamplerState clampSampler : register(s3);
SamplerState pointSampler : register(s4);

struct PSInput
{
    float4 position : SV_POSITION;
    half4 color : COLOR;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float3 binormal : TEXCOORD3;
    float2 texcoord : TEXCOORD4;
};



cbuffer PixelConstantBuffer : register(b1)
{
    float4 data;
    float4 data2;
    float4 materialColor;
};


uint4 main(PSInput input) : SV_Target
{
    
    float4 albedo;

    albedo = data2.x > -1.0 ? float4(1, 1, 1, 1) : albedoMap.Sample(samplerState, input.texcoord);
    albedo *= materialColor;
    
    if (albedo.a < 0.1)
        discard;
    
     
    
    float3 normal = normalize(input.normal);
    float3 tangent = normalize(input.tangent);
    float3 binormal = normalize(cross(normal.xyz, tangent));
    float3x3 tangentSpaceMatrix = float3x3(tangent, binormal, normal.xyz);

    if (data2.y < 0.0)
    {
        float3 normalSample = normalize((normalMap.Sample(samplerState, input.texcoord)).xyz * 2.0f - 1.0f);
        normal.xyz = normalize(mul(normalSample, tangentSpaceMatrix));
    }
    
    float roughness = 0.0;
    float metallic = 0.0;
    float ao = 0.0;
    
    
    
    
    //output.worldPos = float4(input.worldPos, 1.0);
    
    if (data2.w > 0)
    {
        if (data.x > -1.0)
        {
            float4 materialProperties = roughnessMap.Sample(samplerState, input.texcoord);
            
            metallic = 1.0f - materialProperties.r; // Metallic
            roughness = materialProperties.g; // roughness
            ao = materialProperties.b; // AO
        }
        else
        {
            metallic = data.x; // Metallic
            ao = data.y; // AO
            roughness = data.z; // roughness
            
        }
    }
    else
    {
        ao = data.x > -1.0 ? 1.0f : aoMap.Sample(samplerState, input.texcoord).r; // AO
        metallic = 1.0f - (data.z > -1.0 ? data.z : metallicMap.Sample(samplerState, input.texcoord).r); // Metallic
        roughness = data.y > -1.0 ? data.y : roughnessMap.Sample(samplerState, input.texcoord).r; // roughness
    }


    return CreateGBuffer(normal, albedo.xyz, roughness, metallic, ao, 0);
}