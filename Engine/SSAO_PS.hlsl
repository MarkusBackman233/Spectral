#include "common.hlsli"

Texture2D normalMap : register(t0);
Texture2D positionMap : register(t1);
Texture2D depthMap : register(t2);
SamplerState samplerState : register(s3);


struct PSInput
{
    float4 Pos : SV_Position;
    float2 tex : TEXCOORD0;
};


cbuffer PixelConstantBuffer : register(b0)
{ 
    float4x4 viewProjection : Transform;
    float4 settings;
    float4 samples[64];
};




float Hash(float2 p)
{
    float2 K1 = float2(
        23.14069263277926, // e^pi (Gelfond's constant)
         2.665144142690225 // 2^sqrt(2) (Gelfond–Schneider constant)
    );
    return frac(cos(dot(p, K1)) * 12345.6789);

}



float3 RandomNormalizedXY(float2 seed)
{
    float randomX = Hash(seed + settings.w);
    float randomY = Hash(seed + settings.w + float2(1.0f, 0.0)); // Offset the seed slightly for a different result

    // Normalize x and y to the range [-1, 1]
    randomX = randomX * 2.0 - 1.0;
    randomY = randomY * 2.0 - 1.0;

    return float3(randomX, randomY, 0.0);
}


float4 main(PSInput input) : SV_TARGET
{
    if (settings.z == 0)
    {
        return 1.0f;
    }
    float3 worldPos = positionMap.Sample(samplerState, input.tex).xyz;
    float3 normal = normalMap.Sample(samplerState, input.tex).xyz;
    float3 randomVector = normalize(float3(Rand3dTo2d(worldPos), 1.0));

    float3 tangent = normalize(randomVector - normal * dot(randomVector, normal));
    float3 bitangent = cross(normal, tangent);
    float3x3 transformMat = float3x3(tangent, bitangent, normal);
	
    
    float3 center = mul(float4(worldPos, 1.0), viewProjection);

    float radius = settings.z;
    float occlusion = 0.0;
    
    for (int i = 0; i < 64; ++i)
    {
        float3 samplePos = mul(samples[i].xyz, transformMat);
        samplePos = samplePos * radius + worldPos;
        
        float4 offset = mul(float4(samplePos, 1.0), viewProjection);
        offset.xy /= offset.w;
        float sampleDepth = ComputeViewDepth(depthMap.Sample(samplerState, float2(offset.x * 0.5 + 0.5, -offset.y * 0.5 + 0.5)).r);
    
        
        float3 sampleDir = normalize(samplePos - worldPos);

        float nDotS = max(dot(normal, sampleDir), 0);
        
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(center.z - sampleDepth));
        occlusion += (sampleDepth <= offset.z - settings.x*10 ? 1.0 : 0.0) * rangeCheck * nDotS;
    }
    occlusion = 1.0 - (occlusion / 64);
    occlusion = pow(occlusion, settings.y);
    return occlusion;
}