#include "common.hlsli"

Texture2D<uint4> gbufferMap: register(t0);
Texture2D depthMap : register(t1);
Texture2D positionMap : register(t2);
Texture2D shadowDepthMap : register(t3);
TextureCube iradianceMap : register(t4);
TextureCube specularMap : register(t5);
Texture2D specularIntegrationMap : register(t6);
Texture2D SSAOMap : register(t7);


SamplerState samplerState : register(s0);
SamplerComparisonState cmpSampler : register(s1);
SamplerState cubeSampler : register(s2);
SamplerState clampSampler : register(s3);

#define PointLight 0.0f
#define DirectionalLight 1.0f

#define lightType  additionalData.x
#define lightAttenuation  additionalData.y
/*
SamplerState anisotropicSampler
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 16;
    AddressU = Wrap;
    AddressV = Wrap;
    BorderColor = float4(10000, 10000, 10000, 10000);
};

SamplerState clampSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
    BorderColor = float4(10000, 10000, 10000, 10000);
};
*/

struct Light
{
    float4 position;
    float4 direction;
    float4 color;
    float4 additionalData; // x = lightType, y = attenuation, z = enabled
};

cbuffer GlobalPixelConstantBuffer : register(b0)
{
    matrix ViewProjection;
    matrix LightProjection;
    
    matrix Projection;
    
    matrix ViewProjectionInverse;
    matrix ProjectionInverse;
    
    float4 ambientLighting;
    float4 fogColor;
    float3 cameraPosition;
    unsigned int numLights;
    float4 gamma;
    
    Light lights[50];
};
cbuffer SSAOConstantBuffer : register(b5)
{
    float4 ssaoSettings;
    float4 samples[64];
};

#define PI 3.14159265



float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return nom / denom;
}

float GeometrySchlickGGX(float ndotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float nom = ndotV;
    float denom = ndotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(float nDotV, float nDotL, float roughness)
{
    float ggx2 = GeometrySchlickGGX(nDotV, roughness);
    float ggx1 = GeometrySchlickGGX(nDotL, roughness);
    
    return ggx1 * ggx2;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow((1.0 + 0.000001) - cosTheta, 5.0);
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(float3(roughness, roughness, roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float3 computeReflectance(float3 N, float3 V, float3 F0, float3 albedo, float3 L, float3 H, float3 light_col, float intensity, float metallic, float roughness)
{
    float3 radiance = light_col * intensity;
    
    float nDotV = max(dot(H, V), 0.0);
    float nDotL = max(dot(N, L), 0.0);
    
    float3 F = FresnelSchlick(nDotV, F0);
    
    float3 kS = F;
    float3 kD = 1.0f - kS;
    kD *= 1.0 - metallic;
    
    float3 nominator = DistributionGGX(N, H, roughness) * GeometrySmith(nDotV, nDotL, roughness) * F;
    float denominator = 4 * nDotV * nDotL + 0.00001;
    float3 specular = nominator / denominator;

    float3 diffuse_radiance = kD * albedo / PI;
    
    return (diffuse_radiance + specular) * radiance * nDotL;

}

float ShadowCalculation(float4 lpos)
{
    lpos.xyz /= lpos.w;
    if (lpos.x < -1.0f || lpos.x > 1.0f ||
        lpos.y < -1.0f || lpos.y > 1.0f ||
        lpos.z < 0.0f || lpos.z > 1.0f)
    {
        return 1.0;
    }

    lpos.x = lpos.x * 0.5 + 0.5;
    lpos.y = lpos.y * -0.5 + 0.5;
    lpos.z -= 0.001;
    
    const float texelSize = 0.000244140625;
    
    float shadowLevel = 0.0;
    
    float halfSampleDistance = 1.5;
    
    for (float y = -halfSampleDistance; y < halfSampleDistance; y += 1.0)
    {
        for (float x = -halfSampleDistance; x < halfSampleDistance; x += 1.0)
        {
            shadowLevel += shadowDepthMap.SampleCmpLevelZero(cmpSampler, lpos.xy + float2(x, y) * texelSize, lpos.z );

        }
    }
    return shadowLevel * 0.1111111111111111; // 1 / 9
}


float CalculateSSAO(float3 normal, float3 worldPos, float depth, float2 texcoord)
{
    return 1;
    
    float radius = ssaoSettings.z;
    int validSamples = 0;
    float occlusion = 0;
    for (int i = 0; i < 32; ++i)
    {
        float3 samplePos = samples[i].xyz * radius + worldPos;
        
        
        if (dot(samplePos - worldPos, normal) > 0)
        {
            validSamples++;
        }
        else
        {
            continue;
        }
        
        float4 clipPos = mul(float4(samplePos, 1), ViewProjection);
        float3 ndcPos = clipPos.xyz / clipPos.w;
        if (ndcPos.x < -1 || ndcPos.x > 1 || ndcPos.y < -1 || ndcPos.y > 1)
        {
            continue;
        }
        ndcPos.xyz = ndcPos.xyz * 0.5 + 0.5;
        ndcPos.y *= -1;

        float offsetPositionDepth = clipPos.z;
        float sampleDepth = ComputeViewDepth(depthMap.SampleLevel(samplerState, ndcPos.xy, 0).r);
        
        bool notOccluded = sampleDepth < (offsetPositionDepth - ssaoSettings.x);

        if (notOccluded)
        {
            float rangeCheck = smoothstep(0.0, 1.0, radius / abs(offsetPositionDepth - sampleDepth));
            occlusion += 1.0f * rangeCheck;
        }
    }
    
    if (validSamples)
        occlusion /= validSamples;
    else
        occlusion = 1.0;
    
    return pow(1.0 - occlusion, ssaoSettings.y);
}

struct VVSOutput
{
    float4 Pos : SV_Position;
    float2 texcoord : TEXCOORD0;
};

float3 ReconstructWorldPosition(float2 uv, float depth)
{
    float4 ndc = float4(uv * 2.0f - 1.0f, depth, 1.0f);
    ndc.y *= -1.0f;

    float4 viewPos = mul(ViewProjectionInverse, ndc);
    viewPos.xyz /= viewPos.w;

    return viewPos.xyz;
}

float4 main(VVSOutput input) : SV_TARGET
{
    uint4 gbuffer = gbufferMap.Load(int3(input.Pos.xy, 0));
    clip(((float) gbuffer.w) - 0.1);

    float encodedX = float(gbuffer.x & 0x0FFFFFF) / 16777215.0;
    float encodedY = float(gbuffer.y & 0x0FFFFFF) / 16777215.0;
    float2 encoded = float2(encodedX, encodedY);
    
    float3 normal = Decode(encoded);
    
    float3 albedo;
    albedo.x = (float) (gbuffer.z & 0x000000FF) / 255.0f;
    albedo.y = (float) ((gbuffer.z >> 8) & 0x000000FF) / 255.0f;
    albedo.z = (float) ((gbuffer.z >> 16) & 0x000000FF) / 255.0f;
    albedo = pow(albedo, 2.2);
    float roughness =   (float) ((gbuffer.x >> 24) & 0x000000FF) / 255.0f;
    float metallic = 1.0 - (float) ((gbuffer.y >> 24) & 0x000000FF) / 255.0f;
    float ao =          (float) ((gbuffer.z >> 24) & 0x000000FF) / 255.0f;
    
    float depth = depthMap.Load(int3(input.Pos.xy, 0)).r;
    
    bool foliage = (bool) ((gbuffer.w >> 1) & 0x00000001);
    
    
    
    
    float3 worldPosition = ReconstructWorldPosition(input.texcoord, depth);
    float ssao = CalculateSSAO(normal, worldPosition, depth, input.texcoord);
    float3 viewDir = normalize(cameraPosition - worldPosition);
    float3 F0 = lerp(float3(0.03, 0.03, 0.03), albedo, metallic);
    float shadow = ShadowCalculation(mul(float4(worldPosition.xyz, 1.0), LightProjection));
    //shadow = max(shadow, 0.3);
    
    float3 brdfRec = float3(0.0, 0.0, 0.0);
    
    for (uint i = 0; i < numLights; i++)
    {
        if (lights[i].lightType == PointLight)
        {
            float3 distance = worldPosition.xyz - lights[i].position.xyz;
            
            float distanceSquared = distance.x * distance.x + distance.y * distance.y + distance.z * distance.z;
            float intensity = lights[i].lightAttenuation / distanceSquared;
            
            float3 lightDirection = normalize(lights[i].position.xyz - worldPosition.xyz);
            
            float3 H = normalize(viewDir + lightDirection);
            brdfRec += computeReflectance(normal.xyz, viewDir, F0, albedo.rgb, lightDirection, H, lights[i].color.xyz, intensity, metallic, roughness);

        }
        else if (lights[i].lightType == DirectionalLight)
        {
            float3 lightDirection = normalize(-lights[i].direction.xyz);
            float intensity = lights[i].lightAttenuation;

            float3 H = normalize(viewDir + lightDirection);

            brdfRec += computeReflectance(
                normal.xyz,
                viewDir,
                F0,
                albedo.rgb,
                lightDirection,
                H,
                lights[i].color.xyz,
                intensity,
                metallic,
                roughness
            ) * shadow * 0.025;
        }
    }
    
    
    
    float NdotV = max(dot(viewDir, normal), 0.0);
    //float NdotV = abs(dot(viewDir, normal));
    float3 reflectionUnNormalized = reflect(-viewDir, normal);
    float3 F = FresnelSchlickRoughness(NdotV, F0, 1-roughness);
    
    float3 specularSample = specularMap.SampleLevel(samplerState, -reflectionUnNormalized,roughness * 7.0).
    rgb;
    

    float4 brdfTerm = specularIntegrationMap.SampleLevel(clampSampler, float2(NdotV, 1-roughness), 0);
    float3 specular = specularSample * (F * brdfTerm.x + brdfTerm.y);
    float3 iradiance = iradianceMap.SampleLevel(samplerState, normal, 0).rgb / PI;
    if (foliage) iradiance += iradianceMap.SampleLevel(samplerState, -normal, 0).rgb / PI * 0.8;
    
    float3 kS = F;
    float3 kD = (1.0 - kS) * (1.0 - metallic);

    iradiance *= max(ao, 0.3);
    float3 albedoByDiffuse = kD * albedo.rgb * iradiance.rgb;
    

    
    
    float fogHeightMultiplier = clamp(smoothstep(0.0, 1.0, (50.0f - worldPosition.y) * 0.02), 0.0f, 1.0f);
    float fogFactor = (1.0 - exp(-pow(fogColor.a * ComputeViewDepth(depth), 2.0)));
    float3 fog = iradianceMap.SampleLevel(samplerState, -viewDir, 0).rgb * fogFactor * fogHeightMultiplier * fogColor.xyz;
    
    

    
    float3 color = albedoByDiffuse + specular + brdfRec;
    float3 finalColor = color /** shadow */ * ssao + fog;
    
    const float luminance = dot(finalColor, float3(0.2126, 0.7152, 0.0722));
    
    return float4(finalColor, luminance);
}