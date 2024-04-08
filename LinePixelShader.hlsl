cbuffer PixelConstantBuffer : register(b0)
{
    float4 LineColor;
};
SamplerState samplerState : register(s0);

struct PS_INPUT
{
    float4 position : SV_POSITION;
};

struct PS_OUTPUT
{
    float4 color : SV_TARGET;
};

PS_OUTPUT main(PS_INPUT input)
{

    PS_OUTPUT output;
    output.color = LineColor;
    return output;
}