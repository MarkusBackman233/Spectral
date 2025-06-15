TextureCube skyboxMap : register(t0);
TextureCube lastSpecularMap : register(t1);


SamplerState samplerState : register(s0);
SamplerComparisonState cmpSampler : register(s1);
SamplerState cubeSampler : register(s2);
SamplerState clampSampler : register(s3);

struct PS_INPUT
{
    float4 localPos : TEXCOORD1;
    float4 position : SV_POSITION;
};

cbuffer GlobalPixelConstantBuffer : register(b0)
{
    float4 sun;
    float4 data; // x = deltaTime y = roughness
};

static const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
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
// ----------------------------------------------------------------------------
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
// efficient VanDerCorpus calculation.
float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
float2 Hammersley(uint i, uint N)
{
    return float2(float(i) / float(N), RadicalInverse_VdC(i));
}
// ----------------------------------------------------------------------------
float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
    float a = roughness * roughness;
	
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	
	// from spherical coordinates to cartesian coordinates - halfway floattor
    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
	
	// from tangent-space H floattor to world-space sample floattor
    float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangent = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);
	
    float3 samplefloat = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(samplefloat);
}

float Rand3dTo1d(float3 value, float3 dotDir = float3(12.9898, 78.233, 37.719))
{
    float3 smallValue = sin(value);
    float random = dot(smallValue, dotDir);
    random = frac(sin(random) * 143758.5453);
    return random;
}

float2 Rand3dTo2d(float3 value)
{
    return float2(
        Rand3dTo1d(value, float3(12.989, 78.233, 37.719)),
        Rand3dTo1d(value, float3(39.346, 11.135, 83.155))
    );
}


float3 SampleHemisphere(float3 normal, float2 random)
{
    // Scale random.x to control the spread of the samples
    random.x = pow(random.x, 0.1);
    
    float z = sqrt(1.0 - random.x); // Height of the sample
    float r = sqrt(random.x); // Radius of the sample on the XY plane
    float phi = 2.0 * 3.14159265 * random.y; // Angle around the normal

    float x = r * cos(phi); // X-coordinate
    float y = r * sin(phi); // Y-coordinate

    float3 localSample = float3(x, y, z); // Local space sample

    // Calculate tangent and bitangent for transforming to world space
    float3 tangent = normalize(abs(normal.z) > 0.999 ? float3(1, 0, 0) : cross(float3(0, 1, 0), normal));
    float3 bitangent = cross(normal, tangent);

    // Transform sample to world space
    return x * tangent + y * bitangent + z * normal;
}
float3 ImportanceSampleGGX_Randomized(float2 Xi, float3 N, float roughness, float2 randomOffset)
{
    Xi += randomOffset; // Offset Hammersley sampling with random jitter
    Xi = frac(Xi); // Ensure values remain in [0, 1]

    float a = roughness * roughness;

    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangent = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);

    float3 samplefloat = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(samplefloat);
}
float4 main(PS_INPUT input) : SV_TARGET
{
    float3 normal = -normalize(input.localPos.xyz);
    //
    //float3 irradiance = float3(0,0,0);
    //int samples = 80;
    //for (int i = 0; i < samples; i ++)
    //{
    //    irradiance += skyboxMap.SampleLevel(samplerState, SampleHemisphere(-normal, Rand3dTo2d(normal + data.x + i)), 0);
    //}
    //
    //irradiance /= samples;

    float2 randomSeed = Rand3dTo2d(normal + data.x);
    float3 N = normalize(normal);
    // make the simplifying assumption that V equals R equals the normal 
    float3 R = N;
    float3 V = R;

    const uint SAMPLE_COUNT = max(32u * pow(data.y, 0.4), 8u);

    float3 prefilteredColor = float3(0.0,0.0,0.0);
    float totalWeight = 0.0;
    
    
    float resolution = 128.0; // resolution of source cubemap (per face)
    float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
    
    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        // generates a sample floattor that's biased towards the preferred alignment direction (importance sampling).
        float2 Xi = Hammersley(i, SAMPLE_COUNT);
        float3 H = ImportanceSampleGGX_Randomized(Xi, N, data.y, randomSeed*10);
        float3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL > 0.0)
        {
            // sample from the environment's mip level based on roughness/pdf
            float D = DistributionGGX(N, H, data.y);
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;


            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);
            float mipLevel = data.y == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

            prefilteredColor += skyboxMap.SampleLevel(samplerState, L, mipLevel).xyz * NdotL;
            totalWeight += NdotL;
        }
    }
    
    
    
    
    prefilteredColor = prefilteredColor / totalWeight;
    float accumulationTimeInSeconds = 8.4;
    
    //float blendFactor = 1.0 - (min(data.z * 0.00694, accumulationTimeInSeconds) / accumulationTimeInSeconds);
    //blendFactor *= 0.5;
    float blendFactor = 0.01f;
    float3 blendedColor = (blendFactor * prefilteredColor) + ((1.0 - blendFactor) * lastSpecularMap.SampleLevel(samplerState, normal, data.y * 8).xyz);
   // float3 blendedColor = (prefilteredColor + lastSpecularMap.SampleLevel(samplerState, normal, data.y * 8).xyz) / data.z * 144;
    
    
    return float4(blendedColor, 1.00);
    //return float4(skyboxMap.SampleLevel(samplerState, normal, 0).xyz, 0.02);
}
