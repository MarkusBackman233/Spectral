struct VSInput
{
    float3 position : POSITION;
    float2 tex : TEXCOORD;
};

struct PSInput
{
    float4 pos : SV_Position;
    float2 tex : TEXCOORD0;
};

PSInput main(VSInput input)
{
    PSInput output;

    output.pos = float4(input.position, 1.0f);
    output.tex = input.tex;
    return output;
}