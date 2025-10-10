struct VS_INPUT
{
    float3 pos : POSITION; // Required for FXC
    float2 dir : TEXCOORD; // Required for FXC
};
struct VS_OUTPUT
{
    float4 pos : SV_POSITION; // Required for FXC
    float2 dir : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.pos = float4(input.pos, 1.0f); // World space
    output.dir = input.dir;
    return output;
}