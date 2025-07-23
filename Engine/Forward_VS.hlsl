cbuffer VertexConstantBuffer : register(b0)
{
    matrix ViewProjection;
    float4 CameraPosition;
};

struct VS_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
    float2 textureUV : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
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
    output.position = float4(input.position, 1.0);
    output.worldPos = output.position;
    
    output.position = mul(output.position, ViewProjection);
    
    output.normal = input.normal;
    output.normal = normalize(output.normal);
    output.tangent = input.tangent;
    output.tangent = normalize(output.tangent);
    output.binormal = cross(output.normal, output.tangent);
    output.binormal = normalize(output.binormal);
    
    
    output.texcoord = input.textureUV;

    return output;
}