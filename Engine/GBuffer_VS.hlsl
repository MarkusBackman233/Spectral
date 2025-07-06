cbuffer VertexConstantBuffer : register(b0)
{
    matrix ViewProjection;
    float4 CameraPosition;
};

struct VS_INPUT
{
    float3 position : POSITION;
    float2 textureUV : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    row_major float4x4 transform : Transform;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float3 binormal : TEXCOORD3;
    float2 texcoord : TEXCOORD4;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.position = mul(float4(input.position, 1.0), input.transform);
    output.worldPos = output.position;
    
    output.position = mul(output.position, ViewProjection);
    
    output.normal = mul(input.normal, (float3x3) input.transform);
    output.normal = normalize(output.normal);
    output.tangent = mul(input.tangent, (float3x3) input.transform);
    output.tangent = normalize(output.tangent);
    output.binormal = mul(cross(output.normal, output.tangent), (float3x3) input.transform);
    output.binormal = normalize(output.binormal);
    
    
    output.texcoord = input.textureUV;

    return output;
}