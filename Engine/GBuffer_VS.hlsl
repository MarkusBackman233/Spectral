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
    row_major float4x4 transform : Transform;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    half4 color : COLOR;
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
    
    float3x3 normalMatrix = (float3x3) input.transform;
    
    output.normal = mul(input.normal, normalMatrix);
    output.normal = normalize(output.normal);
    output.tangent = mul(input.tangent, normalMatrix);
    output.tangent = normalize(output.tangent);
    output.binormal = mul(cross(output.normal, output.tangent), normalMatrix);
    output.binormal = normalize(output.binormal);
    
    
    output.texcoord = input.textureUV;
    output.color = input.color;
    return output;
}