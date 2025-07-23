TextureCube iradianceMap : register(t4);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    bool isFrontFace : SV_IsFrontFace; // <- built-in semantic
};

SamplerState samplerState : register(s0);
SamplerComparisonState cmpSampler : register(s1);
SamplerState cubeSampler : register(s2);
SamplerState clampSampler : register(s3);
#define PI 3.14159265

float4 main(PS_INPUT input) : SV_TARGET
{
    
    if (!input.isFrontFace)
    {
        input.normal.xyz = -input.normal.xyz;
    }
    float3 frontIrradiance = iradianceMap.SampleLevel(samplerState, input.normal.xyz, 0).rgb / PI;
    
    float3 backIrradiance = iradianceMap.SampleLevel(samplerState, -input.normal.xyz, 0).rgb / PI;
    

    input.color.rgb = input.color.rgb * frontIrradiance + input.color.rgb * backIrradiance * 0.3;
    return float4(input.color.rgb, 1);
}