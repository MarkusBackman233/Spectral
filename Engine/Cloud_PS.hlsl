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
    const float uvScale = 1.0f / 3;
    
    float2 uv = (input.localPos.xz * 0.5 * uvScale) + 0.5;
    
    float4 cloudData = cloudMap.SampleLevel(samplerState, uv, 0);
    
    float3 cloudNormal = cloudData.xyz;
    
    
    
    float3 viewDir = normalize(float3(input.localPos.x, 0.01, input.localPos.z));
    float3 reflection = normalize(reflect(-viewDir, -cloudNormal));
    
    float3 color = skyboxMap.SampleLevel(samplerState, viewDir, cloudData.a * 0.06 * 12).xyz;
    //color *= 1 - cloudData.a * 0.06;
    
    return float4(color, clamp(cloudData.a*0.02,0,1));
}