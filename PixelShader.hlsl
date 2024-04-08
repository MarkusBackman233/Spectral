#define Shininess = data.x;

cbuffer PixelConstantBuffer : register(b0)
{
    float4 ambientLighting;
    float4 fogColor;
    float4 data; // x = Shininess
};

Texture2D albedoTexture : register(t0);
Texture2D normalTexture : register(t1);

SamplerState samplerState : register(s0);

struct PS_INPUT
{
    float4 lpos : TEXCOORD1;
    float4 position : SV_POSITION;
    float2 TextureUV : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float3 viewDirection : VIEW_DIRECTION;
    float distanceToCamera : DISTANCE;
};

struct PS_OUTPUT
{
    float4 color : SV_TARGET;
};

PS_OUTPUT main(PS_INPUT input)
{
    float3 viewDir = normalize(input.viewDirection);
    
    float3 normal = normalize(input.normal);
    float3 tangent = normalize(input.tangent);
    float3 binormal = normalize(cross(normal, tangent));
    float3x3 tangentSpaceMatrix = float3x3(tangent, binormal, normal);
    float3x3 worldToTangentSpace = transpose(tangentSpaceMatrix);
    
    
    float3 normalMap = normalize((normalTexture.Sample(samplerState, input.TextureUV)).xyz * 2.0f - 1.0f);
    normal = normalize(mul(normalMap, tangentSpaceMatrix));
    float3 LightDirection = normalize(float3(0.1, 0.6, 0.9));
    
    float4 albedo = albedoTexture.Sample(samplerState, input.TextureUV);

    
    float3 halfVector = normalize(LightDirection + viewDir);
    float diffuseFactor = max(0.0f, dot(normal, LightDirection));
    float specularFactor = pow(max(0.0f, dot(normal, halfVector)), data.x);
    
    float3 LightColor = float3(1, 1, 1) * ambientLighting.a;
    float3 lightIntensity = LightColor * (diffuseFactor + specularFactor);
    
    lightIntensity = max(lightIntensity, ambientLighting.rgb);
    PS_OUTPUT output;
    float3 finalColor = albedo.rgb * lightIntensity + fogColor.rgb * (input.distanceToCamera * 0.001f);
    
    

    output.color = float4(finalColor, albedo.a);
    return output;

}