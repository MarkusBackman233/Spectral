cbuffer VertexConstantBuffer : register(b0)
{
    matrix ViewProjection;
    matrix LightProjection;
    float4 CameraPosition;
};

struct VS_INPUT
{
    float3 position : POSITION;
    float2 textureUV : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    column_major float4x4 transform : Transform;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 TextureUV : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.position = mul(float4(input.position, 1.0), input.transform);
    output.position = mul(output.position, ViewProjection);
    //output.TextureUV = input.textureUV;
    return output;
}