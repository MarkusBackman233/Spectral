struct VS_OUTPUT
{
    float4 pos : SV_POSITION; // Required for FXC
};

VS_OUTPUT main(float3 pos : POSITION)
{
    VS_OUTPUT output;
    output.pos = float4(pos, 1.0f); // World space
    return output;
}