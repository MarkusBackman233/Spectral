Texture2D albedoMap : register(t0);
Texture2D normalMap : register(t1);
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
    
    float4 ambientLighting;
    float4 fogColor;
    float4 cameraPosition;
    float4 gamma;
    
    Light lights[50];
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
    float r = (roughness + 1.0);
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
    return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
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
    int shadowSamples = 0;
    
    float halfSampleDistance = 1.5;
    
    for (float y = -halfSampleDistance; y < halfSampleDistance; y += 1.0)
    {
        for (float x = -halfSampleDistance; x < halfSampleDistance; x += 1.0)
        {
            shadowLevel += shadowDepthMap.SampleCmpLevelZero(cmpSampler, lpos.xy + float2(x, y) * texelSize, lpos.z );
            shadowSamples++;

        }
    }
    return shadowLevel / shadowSamples;
}

float3 ReinhardTonemap(in float3 color)
{
    return color / (color + 1.0f);
}

float3 ACESFilmicCurve(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0f, 1.0f);
}

struct VVSOutput
{
    float4 Pos : SV_Position;
    float2 texcoord : TEXCOORD0;
};


float4 main(VVSOutput input) : SV_TARGET
{

    float4 normal = normalMap.Sample(samplerState, input.texcoord);
    normal.xyz = normalize(normal.xyz);
    float metallic = normal.w;
    clip(-metallic + 1.0);
    
    float ssao = SSAOMap.Sample(clampSampler, input.texcoord);
    float4 albedo = albedoMap.Sample(samplerState, input.texcoord);
    albedo.rgb = pow(albedo.rgb, 2.2);
    float ao = saturate(albedo.w + ssao);
    
    float4 worldPosition = positionMap.Sample(samplerState, input.texcoord);
    
    float roughness = worldPosition.w;

    float3 viewDir = normalize(cameraPosition.xyz - worldPosition.xyz);
    
    float3 F0 = float3(0.04, 0.04, 0.04);
    
    metallic = 1.0 - metallic;
    
    F0 = lerp(F0, albedo.rgb, metallic);

    
    float3 brdfRec = float3(0.0, 0.0, 0.0);
    float Shadow = ShadowCalculation(mul(float4(worldPosition.xyz, 1.0), LightProjection));
    
    float3 sunDirection = float3(0, -1, 0);
    
    for (float i = 0.0; i < cameraPosition.w; i++)
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
            sunDirection = normalize(-lights[i].direction.xyz);
        //    float intensity = lights[i].lightAttenuation * 0.01;
        //    float3 lightDirection = normalize(-lights[i].direction.xyz);
        //    float3 H = normalize(viewDir + lightDirection);
        //    brdfRec += computeReflectance(normal.xyz, viewDir, F0, albedo.rgb, -lights[i].direction.xyz, H, lights[i].color.xyz, intensity, metallic, roughness) * Shadow;
        }
    }
    
    
    
    float NdotV = max(dot(viewDir, normal.xyz), 0.0);
    float3 reflection = normalize(reflect(-viewDir, normal.xyz));
    float3 F = FresnelSchlickRoughness(NdotV, F0, roughness);
    
    float3 specularSample = specularMap.SampleLevel(samplerState, -reflection, roughness * 8).rgb;
    

    float4 brdfTerm = specularIntegrationMap.SampleLevel(clampSampler, float2(NdotV, 1.0 - roughness), 0);
    float3 specular = specularSample * (F * brdfTerm.x + brdfTerm.y) * ao;
    float3 iradiance = iradianceMap.SampleLevel(samplerState, normal.xyz, 0).rgb / PI;
    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    
    iradiance = lerp(iradiance * 0.3f, iradiance, ao);
    float3 albedoByDiffuse = kD * albedo.rgb * iradiance.rgb;
    float3 fog = length(worldPosition.xyz - cameraPosition.xyz) * fogColor.xyz * 0.01f * fogColor.a;
    float3 color = (albedoByDiffuse + specular + brdfRec * ao) * ssao * max(Shadow,0.3) + fog;
    const float luminance = dot(color, float3(0.2126, 0.7152, 0.0722));
    
    return float4(color, luminance);
}