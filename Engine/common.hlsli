float2 OctWrap(float2 v)
{
    return (1.0 - abs(v.yx)) * (v.xy >= 0.0 ? 1.0 : -1.0);
}
 
float2 Encode(float3 n)
{
    n /= (abs(n.x) + abs(n.y) + abs(n.z));
    n.xy = n.z >= 0.0 ? n.xy : OctWrap(n.xy);
    n.xy = n.xy * 0.5 + 0.5;
    return n.xy;
}
 
float3 Decode(float2 f)
{
    f = f * 2.0 - 1.0;
 
    // https://twitter.com/Stubbesaurus/status/937994790553227264
    float3 n = float3(f.x, f.y, 1.0 - abs(f.x) - abs(f.y));
    float t = saturate(-n.z);
    n.xy += n.xy >= 0.0 ? -t : t;
    return normalize(n);
}


uint4 CreateGBuffer(in const float3 normal, in const float3 albedo, in const float roughness, in const float metallic, in const float ao, in const uint foliage)
{
    uint4 output = 0;
    
    float2 encodedNormal = Encode(normal) * 16777215;
    output.x |= (uint) encodedNormal.x;
    output.y |= (uint) encodedNormal.y;
    
    output.z |= (uint) (albedo.x * 255);
    output.z |= ((uint) (albedo.y * 255) << 8);
    output.z |= ((uint) (albedo.z * 255) << 16);
    
    output.x |= (uint) (roughness * 255.0) << 24;
    output.y |= (uint) (metallic * 255.0) << 24;
    output.z |= (uint) (ao * 255.0) << 24;
    
    output.w = 1;
    
    output.w |= (foliage << 1);
    
    return output;
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

float ComputeViewDepth(float z)
{
    float zNear = 0.1;
    float zFar = 1000.0;
    return zNear * zFar / (zFar + z * (zNear - zFar));
}