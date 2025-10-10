cbuffer ViewConstantBuffer : register(b0)
{
    matrix ViewProjection;
    float4 CameraPosition;
    float CB_elapsedTime;
    float CB_unused1;
    float CB_unused2;
    float CB_unused3;
};

struct GSInput
{
    float4 pos : SV_POSITION;
    float2 dir : TEXCOORD0;
};

struct GSOutput
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
};


float Rand3dTo1d(float3 value, float3 dotDir = float3(12.9898, 78.233, 37.719))
{
    float random = dot(value, dotDir);
    random = frac(sin(random) * 143758.5453);
    return random;
}

float2 Rand3dTo2d(float3 value)
{
    float3 smallValue = sin(value);
    
    return float2(
        Rand3dTo1d(smallValue, float3(12.989, 78.233, 37.719)),
        Rand3dTo1d(smallValue, float3(39.346, 11.135, 83.155))
    );
}


GSOutput GenerateVertex(float width, float3 pos, float3 right, float4 color, float3 normal)
{
    GSOutput output;

    output.pos = float4(pos, 1);
    
    output.pos.xyz += right * width;
    output.pos = mul(output.pos, ViewProjection);
    output.color = color;
    output.normal = normal;
    
    return output;
}


float3 BezierQuadratic(float3 p0, float3 p1, float3 p2, float t)
{
    float3 a = lerp(p0, p1, t);
    float3 b = lerp(p1, p2, t);
    return lerp(a, b, t);
}

float3 BezierQuadraticTangent(float3 p0, float3 p1, float3 p2, float t)
{
    return 2. * (1. - t) * (p1 - p0) + 2. * t * (p2 - p1);
}

float3 RandomUnitVectorXZ(float3 seed)
{
    float angle = Rand3dTo1d(seed) * 6.2831853;
    return float3(cos(angle), 0.0, sin(angle));
}


[maxvertexcount(12)]
void main(point GSInput input[1], inout TriangleStream<GSOutput> TriStream)
{

    
    
    
    float4 center = input[0].pos;
    
    float2 rand = Rand3dTo2d(center.xyz);
    float s = rand.x + 0.4;
    
    float3 front = float3(input[0].dir.x, 0, input[0].dir.y);
    float3 right = cross(float3(0, 1, 0),front);
    
    
    float width = 0.03f * s;
    float height = 2.0f * s;
    
    
    float3 wind = float3(0.4 * cos(CB_elapsedTime + rand.y), 0.3, -0.1) * sin(CB_elapsedTime + rand.x) * 1;
    
    float3 p0 = center;
    float3 p1 = p0 + float3(0, height, 0) + wind * s * 0.5;
    float3 p2 = p1 + front * 1.2 * s + wind * s;
    
    
    
    


    
    float4 lightGreenColor = float4(0.32, 0.61, 0.0144, 0.8f);
    float4 darkestGreenColor = float4(0.117, 0.300, 0.0144, 1.0f);
    
    float d = length(CameraPosition.xz - center.xz);
    float c_d = min(d, 10) * 0.1 + 1.0f;
    
    
    int maxSegments = 6;
    
    if (d > 50)
    {
        maxSegments = 2;
    }
    else if (d > 25)
    {
        maxSegments = 3;
    }
    else if (d > 20)
    {
        maxSegments = 4;
    }
    
    for (int i = 0; i < maxSegments; i++)
    {
        float fraction = (float) i / maxSegments;
    
        float3 p = BezierQuadratic(p0, p1, p2, fraction);
        float3 tangent = BezierQuadraticTangent(p0, p1, p2, fraction);
        float4 color = lerp(darkestGreenColor, lightGreenColor, fraction);
    
        float w = width * smoothstep(1, 0.2, fraction);
        float3 normal = cross(right, normalize(tangent));
        //normal.xyz = lerp(normal.xyz, float3(0, 1, 0), 0.8);
    
        
        GSOutput v1 = GenerateVertex(w, p, right * c_d, color, normal);
        GSOutput v2 = GenerateVertex(w, p, -right * c_d, color, normal);

        TriStream.Append(v1);
        TriStream.Append(v2);
    }
}