cbuffer VertexConstantBuffer : register(b0)
{
    matrix ViewProjection;
};

struct VS_INPUT
{
    float3 position : POSITION;
    float2 tex : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.position = mul(float4(input.position, 1.0), ViewProjection);
    output.tex = input.tex;
    return output;
}