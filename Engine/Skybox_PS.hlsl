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


float3 ReinhardTonemap(in float3 color)
{
    color *= 8;
    color = color / (1 + color);
    return color;
}
float4 main(PS_INPUT input) : SV_TARGET
{
    float3 fragmentDirection = normalize(input.localPos.xyz);
    float fragmentHeight = max(dot(fragmentDirection, float3(0, 1, 0)), 0.0);
    
    if (sun.w == 1.0f)
    {
        float3 sunDir = normalize(sun.xyz);
        
        
        
        
        float3 sunColor = float3(1,0,0);
        float sunHeight = max(dot(sunDir, -float3(0, 1, 0)), 0.0);
        {
            int s = sign(sunHeight);
            sunHeight *= s;
            sunHeight = saturate(sunHeight);
            sunColor = lerp(float3(1.00000, 0.31765, 0.00392), float3(1.0, 1.0, 1.0), pow(sunHeight,0.5));
        }

        float3 deepColor = float3(0.00092, 0.29216, 0.62549);
        float3 lightColor = float3(0.898, 0.902, 0.922);

        
        float3 color;
        { // atmosphere
            int s = sign(fragmentHeight);

            fragmentHeight = saturate(fragmentHeight);
            
            float sunRotation = dot(normalize(input.localPos.xz), -normalize(sun.xz));
            
            float3 horizonColor = lerp(lightColor * (1.0 - (sunHeight * 0.5 + 0.1)), sunColor,  (sunHeight)+sunRotation * 0.2);
            color = lerp(horizonColor, deepColor, fragmentHeight);
        }
        
        { // sun
            //float factor = max(dot(fragmentDirection, -sunDir), 0.0);
            float factor = (dot(fragmentDirection, -sunDir) + 1.0f) / 2;
            color += sunColor * factor * 0.3;
            int s = sign(factor);
            
            float uHeight = max(fragmentHeight, 0.001);
            
            factor = pow(factor, 9050.0 / (1.0 / pow(uHeight,1.15)));
            factor *= s;
            factor = saturate(factor);
            color += sunColor * factor * 50 * pow(uHeight, 0.99);
        }
        
        

        color = pow(max(color * pow(fragmentHeight + 0.005, 0.2), color * pow(0.005, 0.2)), 2.2);
        color *= (1.0f - clamp(sunDir.y * 2, 0.0, 1.0));
        
        return float4(color, 1.0);
    }
    fragmentDirection.z = -fragmentDirection.z;
    return float4(skyboxMap.SampleLevel(samplerState, fragmentDirection, 0).rgb, 1.0f);
}