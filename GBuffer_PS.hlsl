Texture2D albedoMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D roughnessMap : register(t2);
Texture2D metallicMap : register(t3);
Texture2D aoMap : register(t4);

SamplerState samplerState : register(s0);
SamplerComparisonState cmpSampler : register(s1);
SamplerState cubeSampler : register(s2);
SamplerState clampSampler : register(s3);

struct PSInput
{
    float4 position : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float3 binormal : TEXCOORD3;
    float2 texcoord : TEXCOORD4;
    float4 lpos : TEXCOORD5;
};

struct PSOutput
{
    float4 albedo : SV_Target0;
    float4 normal : SV_Target1;
    float4 worldPos : SV_Target2;
    float4 lightPos : SV_Target3;
};

cbuffer PixelConstantBuffer : register(b1)
{
    float4 data;
    float4 data2;
    float4 materialColor;
};

PSOutput main(PSInput input)
{
    PSOutput output;
    
    output.albedo = data2.x > -1.0 ? float4(1, 1, 1, 1) : albedoMap.Sample(samplerState, input.texcoord);
    output.albedo *= materialColor;
    
    if (output.albedo.a < 0.1)
        discard;
    
    output.normal.xyz = normalize(input.normal);
    float3 tangent = normalize(input.tangent);
    float3 binormal = normalize(cross(output.normal.xyz, tangent));
    float3x3 tangentSpaceMatrix = float3x3(tangent, binormal, output.normal.xyz);

    if (data2.y < 0.0)
    {
        float3 normalSample = normalize((normalMap.Sample(samplerState, input.texcoord)).xyz * 2.0f - 1.0f);
        output.normal.xyz = normalize(mul(normalSample, tangentSpaceMatrix));
    }
    output.worldPos = float4(input.worldPos, 1.0);
    

    output.albedo.w = data.x > -1.0 ? 1.0f : aoMap.Sample(samplerState, input.texcoord).r; // AO
    output.normal.w = 1.0f - (data.z > -1.0 ? data.z : metallicMap.Sample(samplerState, input.texcoord).r); // Metallic
    output.worldPos.w = data.y > -1.0 ? data.y : roughnessMap.Sample(samplerState, input.texcoord).r; // roughness
    output.lightPos = input.lpos;
    return output;
}