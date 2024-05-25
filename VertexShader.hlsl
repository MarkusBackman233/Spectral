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
    float4 lpos : TEXCOORD1;
    float4 position : SV_POSITION;
    float4 worldPosition : TEXCOORD2;
    float2 TextureUV : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float3 viewDirection : VIEW_DIRECTION;
    float distanceToCamera : DISTANCE;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.position = mul(float4(input.position, 1.0), input.transform);
    output.worldPosition = output.position;
    output.viewDirection = normalize(CameraPosition.xyz - output.position.xyz);
    output.distanceToCamera = length(CameraPosition.xyz - output.position.xyz);

    output.lpos = mul(float4(input.position, 1.0), input.transform);
    output.lpos = mul(output.lpos, LightProjection);
    
    output.position = mul(output.position, ViewProjection);
    
    output.normal = mul(input.normal, (float3x3) input.transform);
    output.normal = normalize(output.normal);

    output.tangent = mul(input.tangent, (float3x3) input.transform);
    output.tangent = normalize(output.tangent);

    output.binormal = mul(cross(output.normal, output.tangent), (float3x3) input.transform);
    output.binormal = normalize(output.binormal);
    
    
    output.TextureUV = input.textureUV;

    return output;
}