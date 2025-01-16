TextureCube skyboxMap : register(t0);


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

float3 ReinhardTonemap(in float3 color)
{
    color *= 8;
    color = color / (1 + color);
    return color;
}
float Rand3dTo1d(float3 value, float3 dotDir = float3(12.9898, 78.233, 37.719))
{
    //make value smaller to avoid artefacts
    float3 smallValue = sin(value);
    //get scalar value from 3d vector
    float random = dot(smallValue, dotDir);
    //make value more random by making it bigger and then taking teh factional part
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
    float z = sqrt(1.0 - random.x); // Cosine-weighted elevation
    float r = sqrt(random.x); // Radius on the unit circle
    float phi = 2.0 * 3.14159265 * random.y; // Azimuth angle

    float x = r * cos(phi);
    float y = r * sin(phi);

    // Local space sample
    float3 localSample = float3(x, y, z);

    // Convert to world space
    float3 tangent = normalize(abs(normal.z) > 0.999 ? float3(1, 0, 0) : cross(float3(0, 1, 0), normal));
    float3 bitangent = cross(normal, tangent);
    return x * tangent + y * bitangent + z * normal;
}
float4 main(PS_INPUT input) : SV_TARGET
{
    float3 normal = -normalize(input.localPos.xyz);
    
    float3 irradiance = float3(0,0,0);
    //
    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = normalize(cross(up, normal));
    up = normalize(cross(normal, right));

    //float sampleDelta = 0.5;
    //float nrSamples = 0.0;
    //for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    //{
    //    for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
    //    {
    //        float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
    //        float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;
    //
    //        irradiance += skyboxMap.SampleLevel(samplerState, -sampleVec, 0) * cos(theta) * sin(theta);
    //        nrSamples++;
    //    }
    //}
    //irradiance = PI * irradiance * (1.0 / float(nrSamples));
    
    
    float sampleDelta = 0.1;
    float nrSamples = 0.0;
    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;
    
            irradiance += skyboxMap.SampleLevel(samplerState, -sampleVec, 6) * cos(theta) * sin(theta);
            nrSamples++;  
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    //int samples = 1024;
    //for (int i = 0; i < samples; i ++)
    //{
    //    irradiance += skyboxMap.SampleLevel(samplerState, SampleHemisphere(-normal, Rand3dTo2d(normal + data.x + i)), 0);
    //}
    
    //irradiance /= nrSamples;
    return float4(irradiance, 0.2);
}
