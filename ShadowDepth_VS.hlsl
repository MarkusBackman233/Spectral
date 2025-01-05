cbuffer ConstantBuffer : register(b0)
{
    matrix LightViewProjection;
};

struct VS_INPUT
{
    float3 position : POSITION;
    row_major float4x4 transform : Transform;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.position = mul(float4(input.position, 1.0), input.transform);
    output.position = mul(output.position, LightViewProjection);
    return output;
}