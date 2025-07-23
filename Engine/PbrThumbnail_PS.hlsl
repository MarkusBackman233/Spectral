Texture2D albedoMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D roughnessMap : register(t2);
Texture2D metallicMap : register(t3);
Texture2D aoMap : register(t4);

SamplerState samplerState : register(s0);
SamplerComparisonState cmpSampler : register(s1);
SamplerState cubeSampler : register(s2);
SamplerState clampSampler : register(s3);

struct PS_INPUT
{
    float4 position : SV_POSITION;
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

float4 main(PS_INPUT input) : SV_TARGET
{
    
    float4 albedo = float4(albedoMap.Sample(samplerState, input.texcoord).rgb, 1.0);
    
    
    float3 normal = normalize(input.normal);
    float3 tangent = normalize(input.tangent);
    float3 binormal = normalize(cross(normal.xyz, tangent));
    float3x3 tangentSpaceMatrix = float3x3(tangent, binormal, normal.xyz);

    if (data2.y < 0.0)
    {
        float3 normalSample = normalize((normalMap.Sample(samplerState, input.texcoord)).xyz * 2.0f - 1.0f);
        normal.xyz = normalize(mul(normalSample, tangentSpaceMatrix));
    }
    
    float d = (dot(normal, normalize(float3(0.5f, 1.0f, 0.0f))) + 1.0f) * 0.5f;
    
    return albedo * d;
}