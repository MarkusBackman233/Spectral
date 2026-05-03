Texture2D HeightMap : register(t0);
Texture2D HeightMap2 : register(t1);

SamplerState samplerState : register(s0);

cbuffer VertexConstantBuffer : register(b0)
{
    matrix ViewProjection;
    float4 CameraPosition;
};

cbuffer TerrainVertexConstantBuffer : register(b1)
{
    float WorldSize;
    float WorldMaxHeight;
    float StartX;
    float StartZ;
};



struct VS_INPUT
{
    float2 localPosition : POSITION;
    float2 instancePosition : POSITIONINST;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float3 binormal : TEXCOORD3;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    float3 pos = float3(input.localPosition.x + StartX, 0.0f, input.localPosition.y + StartZ);
    int2 coord = int2(input.localPosition);
    float h = HeightMap.Load(int3(coord, 0)).r;
    pos.y = h * WorldMaxHeight - WorldMaxHeight * 0.5f;
    pos.y += length(HeightMap2.SampleLevel(samplerState, pos.xz / WorldSize,0).r)*20.0f;
    
    output.worldPos = pos;
    output.position = mul(float4(pos,1.0), ViewProjection);
    
    output.normal = float3(0, 1, 0);
    output.tangent = float3(1, 0, 0);
    output.binormal = normalize(cross(output.normal, output.tangent));
    return output;
}