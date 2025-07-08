cbuffer VertexConstantBuffer : register(b0)
{
    matrix ViewProjection;
    float4 CameraPosition;
};

struct VS_INPUT
{
    float3 position : POSITION;
};

struct VS_OUTPUT
{
    float4 localPos : TEXCOORD1;
    float4 position : SV_POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.position = mul(float4(input.position + CameraPosition.xyz, 1.0), ViewProjection);
    output.localPos.xyz = input.position.xyz;

    return output;
}