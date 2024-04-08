
struct Material
{
    float3 albedo;
    float metallic;
    float roughness;
};

struct Light
{
    float3 position;
    float3 color;
};

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


SamplerState samplerState : register(s0);
SamplerComparisonState cmpSampler : register(s1);


Texture2D albedoMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D roughnessMap : register(t2);
Texture2D metallicMap : register(t3);
Texture2D aoMap : register(t4);

Texture2D shadowDepthMap : register(t5);


cbuffer PixelConstantBuffer : register(b0)
{
    float4 ambientLighting;
    float4 fogColor;
    float4 data; // x = hasAO 
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

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow((1.0 + 0.000001 /*avoid negative approximation when cosTheta = 1*/) - cosTheta, 5.0);
}

float3 computeReflectance(float3 N, float3 Ve, float3 F0, float3 albedo, float3 L, float3 H, float3 light_col, float intensity, float metallic, float roughness)
{
    float3 radiance =  light_col * intensity; //Incoming Radiance

    // cook-torrance brdf
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, Ve, L,roughness);
    float3 F    = fresnelSchlick(max(dot(H, Ve), 0.0), F0);

    float3 kS = F;
    float3 kD = float3(1.0,1.0,1.0) - kS;
    kD *= 1.0 - metallic;

    float3 nominator    = NDF * G * F;
    float denominator = 4 * max(dot(N, Ve), 0.0) * max(dot(N, L), 0.0) + 0.00001/* avoid divide by zero*/;
    float3 specular     = nominator / denominator;

    // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);
    float3 diffuse_radiance = kD * (albedo) / PI;

    return (diffuse_radiance + specular) * radiance * NdotL;
}

float ShadowCalculation(float4 lpos)
{
    lpos.xyz /= lpos.w;

    // If position is not visible to the light - don't illuminate it
    // Results in hard light frustum
    if (lpos.x < -1.0f || lpos.x > 1.0f ||
        lpos.y < -1.0f || lpos.y > 1.0f ||
        lpos.z < 0.0f || lpos.z > 1.0f)
    {
        return 1.0;
    }

    // Transform clip space coords to texture space coords (-1:1 to 0:1)
    lpos.x = lpos.x * 0.5 + 0.5;
    lpos.y = lpos.y * -0.5 + 0.5;
    lpos.z -= 0.001;
    
    const float texelSize = 0.000244140625;
    
    float shadowLevel = 0.0;
    for (float y = -1.5; y < 1.5; y += 1.0)
    {
        for (float x = -1.5; x < 1.5; x += 1.0)
        {
            shadowLevel += shadowDepthMap.SampleCmpLevelZero(cmpSampler, lpos.xy + float2(x, y) * texelSize, lpos.z);
        }
    }
    return shadowLevel / 9;
}



float4 main(PS_INPUT input) : SV_TARGET
{
    float4 albedo = albedoMap.Sample(samplerState, input.TextureUV);
    float3 viewDir = normalize(input.viewDirection);

    float3 normal = normalize(input.normal);
    float3 tangent = normalize(input.tangent);
    float3 binormal = normalize(cross(normal, tangent));
    float3x3 tangentSpaceMatrix = float3x3(tangent, binormal, normal);
    float3x3 worldToTangentSpace = transpose(tangentSpaceMatrix);

    float3 normalSample = normalize((normalMap.Sample(samplerState, input.TextureUV)).xyz * 2.0f - 1.0f);
    normal = normalize(mul(normalSample, tangentSpaceMatrix));
    float3 LightDirection = -normalize(float3(0.1f, -0.6f, -0.9f));
    
    float3 materialAO = aoMap.Sample(samplerState, input.TextureUV).rgb;
    float3 ambient = ambientLighting.xyz * albedo.rgb * max(materialAO, data.x);
    
    //Average F0 for dielectric materials
    float3 F0 = float3(0.04, 0.04, 0.04);
    // Get Proper F0 if material is not dielectric
    
    float materialMetallic = metallicMap.Sample(samplerState, input.TextureUV).r * data.z;
    float materialRoughness = roughnessMap.Sample(samplerState, input.TextureUV).r * data.y;
    
    float3 lightColor = fogColor.xyz ;
    
    F0 = lerp(albedo.rgb, F0, materialMetallic);

    float intensity = fogColor.a * 10;
    //if (l.type == 1)
    //{
    //    float l_dist = lightDist(hit.hit_point, l);
    //    intensity = l.intensity / (l_dist * l_dist);
    //} 

    float3 H = normalize(viewDir + LightDirection);
    float Shadow = ShadowCalculation(input.lpos);
    float3 brdfRec = computeReflectance(normal, viewDir, F0, albedo.rgb, LightDirection, H, lightColor, intensity, materialMetallic, materialRoughness)
    * Shadow + fogColor.rgb * (input.distanceToCamera * 0.0001f);
    
    if (albedo.a < 0.1)
        discard;
    return float4(pow(ambient + brdfRec, 1.0 / 2.2), 1.0);
}