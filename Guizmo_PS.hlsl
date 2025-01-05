Texture2D guizmoTexture : register(t0);

cbuffer PixelConstantBuffer : register(b0)
{
    float4 LineColor;
};
SamplerState samplerState : register(s0);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD;
};

struct PS_OUTPUT
{
    float4 color : SV_TARGET;
};

PS_OUTPUT main(PS_INPUT input)
{
    PS_OUTPUT output;
    
    output.color = guizmoTexture.Sample(samplerState, input.tex) * LineColor;
    if (output.color.a < 0.1)
        discard;
    return output;
}