Texture2D albedo0Map : register(t0);
Texture2D albedo1Map : register(t1);
Texture2D albedo2Map : register(t2);
Texture2D albedo3Map : register(t3);

Texture2D normal0Map : register(t4);
Texture2D normal1Map : register(t5);
Texture2D normal2Map : register(t6);
Texture2D normal3Map : register(t7);

Texture2D roughness0Map : register(t8);
Texture2D roughness1Map : register(t9);
Texture2D roughness2Map : register(t10);
Texture2D roughness3Map : register(t11);

SamplerState samplerState : register(s0);
SamplerComparisonState cmpSampler : register(s1);
SamplerState cubeSampler : register(s2);
SamplerState clampSampler : register(s3);
SamplerState pointSampler : register(s4);

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float3 binormal : TEXCOORD3;
    float2 texcoord : TEXCOORD4;
};

struct PSOutput
{
    float4 albedo : SV_Target0;
    float4 normal : SV_Target1;
    float4 worldPos : SV_Target2;
};

cbuffer PixelConstantBuffer : register(b1)
{
    float3 CB_mouseRaycastHit;
    float CB_brushSize;
    unsigned int CB_materialData;
    float unused1;
    float unused2;
    float unused3;
};

#define Albedo0  0
#define Albedo1  1
#define Albedo2  2
#define Albedo3  3
#define Normal0  4
#define Normal1  5
#define Normal2  6
#define Normal3  7
#define Roughness0 8
#define Roughness1 9
#define Roughness2 10
#define Roughness3 11



float4 BlendTextures4(float4 textures0, float4 textures1, float4 textures2, float4 textures3, float4 splat)
{
    return textures0 * splat.r +
           textures1 * splat.g +
           textures2 * splat.b +
           textures3 * splat.a;
}

float3 BlendTextures3(float3 textures0, float3 textures1, float3 textures2, float3 textures3, float4 splat)
{
    return textures0.rgb * splat.r +
           textures1.rgb * splat.g +
           textures2.rgb * splat.b +
           textures3.rgb * splat.a;
}

float BlendTextures1(float textures0, float textures1, float textures2, float textures3, float4 splat)
{
    return textures0 * splat.r +
           textures1 * splat.g +
           textures2 * splat.b +
           textures3 * splat.a;
}

float4 GetTexture4(Texture2D textureMap, float2 texcoord, int bit)
{
    if (CB_materialData & (1 << bit))
    {
        return textureMap.Sample(samplerState, texcoord * 0.3);
    }
    
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

float3 GetTexture3(Texture2D textureMap, float2 texcoord, int bit)
{
    if (CB_materialData & (1 << bit))
    {
        return textureMap.Sample(samplerState, texcoord * 0.3).rgb;
    }
    
    return float3(0.0f, 0.0f, 0.0f);
}

float GetTexture1(Texture2D textureMap, float2 texcoord, int bit)
{
    if (CB_materialData & (1 << bit))
    {
        return textureMap.Sample(samplerState, texcoord*0.3).r;
    }
    
    return 0.0f;
}


PSOutput main(PSInput input)
{
    PSOutput output;
    
    
    //float4 splat = splatMap.Sample(samplerState, input.texcoord);
    float4 splat = input.color;
    

    
    float weightSum = splat.r + splat.g + splat.b + splat.a;
    splat /= max(weightSum, 0.0001);
    
    
    

    float4 blendedAlbedo = BlendTextures4(
        GetTexture4(albedo0Map, input.texcoord, Albedo0),
        GetTexture4(albedo1Map, input.texcoord, Albedo1),
        GetTexture4(albedo2Map, input.texcoord, Albedo2),
        GetTexture4(albedo3Map, input.texcoord, Albedo3),
        splat
    );
    
    float3 blendedNormal = BlendTextures3(
        GetTexture3(normal0Map, input.texcoord, Normal0),
        GetTexture3(normal1Map, input.texcoord, Normal1),
        GetTexture3(normal2Map, input.texcoord, Normal2),
        GetTexture3(normal3Map, input.texcoord, Normal3),
        splat
    );
    
    
    float roughness0 = 0.0f;
    float roughness1 = 0.0f;
    float roughness2 = 0.0f;
    float roughness3 = 0.0f;
    
    float metallic0 = 0.0f;
    float metallic1 = 0.0f;
    float metallic2 = 0.0f;
    float metallic3 = 0.0f;
    
    float ao0 = 0.0f;
    float ao1 = 0.0f;
    float ao2 = 0.0f;
    float ao3 = 0.0f;
    
    

    if (CB_materialData & (1 << Roughness0))
    {
        float3 multimaterialProperties = roughness0Map.Sample(samplerState, input.texcoord).rgb;
        metallic0 = multimaterialProperties.r;
        roughness0 = multimaterialProperties.g;
        ao0 = multimaterialProperties.b;
    }
    if (CB_materialData & (1 << Roughness1))
    {
        float3 multimaterialProperties = roughness1Map.Sample(samplerState, input.texcoord).rgb;
        metallic1 = multimaterialProperties.r;
        roughness1 = multimaterialProperties.g;
        ao1 = multimaterialProperties.b;
    }
    if (CB_materialData & (1 << Roughness2))
    {
        float3 multimaterialProperties = roughness2Map.Sample(samplerState, input.texcoord).rgb;
        metallic2 = multimaterialProperties.r;
        roughness2 = multimaterialProperties.g;
        ao2 = multimaterialProperties.b;
    }
    if (CB_materialData & (1 << Roughness3))
    {
        float3 multimaterialProperties = roughness3Map.Sample(samplerState, input.texcoord).rgb;
        metallic3 = multimaterialProperties.r;
        roughness3 = multimaterialProperties.g;
        ao3 = multimaterialProperties.b;
    }

    output.normal.w = BlendTextures1(metallic0, metallic1, metallic2, metallic3, splat); // Metallic
    output.albedo.w = BlendTextures1(ao0, ao1, ao2, ao3, splat); // AO
    output.worldPos.w = BlendTextures1(roughness0, roughness1, roughness2, roughness3, splat); // roughness
    
    
    output.albedo = float4(blendedAlbedo.rgb, 1.0f);

    
    float d = length(input.worldPos.xz - CB_mouseRaycastHit.xz);
    
    const float lineThickness = 0.4;
    if (d > CB_brushSize && d < CB_brushSize + lineThickness)
    {
        output.albedo = float4(0, 0, 1, 1.0f);
        output.normal.w = 0.0; // Metallic
        output.worldPos.w = 0.05; 
    }

    //output.albedo.rgb = input.color.rgb;
    output.normal.xyz = normalize(input.normal);
    float3 tangent = normalize(input.tangent);
    float3 binormal = normalize(cross(output.normal.xyz, tangent));
    float3x3 tangentSpaceMatrix = float3x3(tangent, binormal, output.normal.xyz);

    uint normalMask = (1 << Normal0) | (1 << Normal1) | (1 << Normal2) | (1 << Normal3);
    if (CB_materialData & normalMask)
    {
        float3 normalSample = normalize(blendedNormal.xyz * 2.0f - 1.0f);
        output.normal.xyz = normalize(mul(normalSample, tangentSpaceMatrix));
    }
    

    output.worldPos = float4(input.worldPos, 1.0);
    

    
    return output;
}