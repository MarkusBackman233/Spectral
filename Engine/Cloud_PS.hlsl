TextureCube skyboxMap : register(t0);
Texture2D cloudMap : register(t1);

SamplerState samplerState : register(s0);
SamplerComparisonState cmpSampler : register(s1);
SamplerState cubeSampler : register(s2);
SamplerState clampSampler : register(s3);

struct PS_INPUT
{
    float4 localPos : TEXCOORD1;
    float4 position : SV_POSITION;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    const float uvScale = 1.0f / 100;
    
    float4 cloudData = cloudMap.SampleLevel(samplerState, ((input.localPos.xz * 0.5 * uvScale) + 0.5), 0);
    float normDepth = cloudData.a /= 5.0f;
    //clip(depth);
    
    float3 cloudNormal = cloudData.xyz;
    
    
    float3 color = skyboxMap.SampleLevel(samplerState, cloudNormal, normDepth * 12).xyz;
     //color += skyboxMap.SampleLevel(samplerState, cloudNormal, 2).xyz;
    
    return float4(color, normDepth);
}