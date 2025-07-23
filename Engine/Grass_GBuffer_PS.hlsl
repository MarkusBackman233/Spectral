
struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    bool isFrontFace : SV_IsFrontFace;
};

SamplerState samplerState : register(s0);
SamplerComparisonState cmpSampler : register(s1);
SamplerState cubeSampler : register(s2);
SamplerState clampSampler : register(s3);
#define PI 3.14159265

struct PSOutput
{
    float4 albedo : SV_Target0;
    float4 normal : SV_Target1;
    float4 worldPos : SV_Target2;
};


PSOutput main(PS_INPUT input) : SV_TARGET
{
    
    PSOutput output;
    
    if (!input.isFrontFace)
    {
        input.normal.xyz = -input.normal.xyz;
    }
    input.normal.xyz = lerp(input.normal.xyz, float3(0, 1, 0), 0.2);

    output.albedo.rgb = input.color.rgb;
    output.normal.xyz = normalize(input.normal.xyz);
    output.worldPos.xyz = input.worldPosition;
    
    
    output.normal.w = 1.0f; // Metallic
    output.worldPos.w = 0.6f; // roughness
    output.albedo.w = output.albedo.a; // AO
    
    
    return output;
}