struct VS_INPUT
{
    float3 pos : POSITION;
    float size : SIZE;
    matrix transform : Transform;
};
struct VS_OUTPUT
{
    float3 pos : POSITION;
    nointerpolation float size : SIZE;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.pos = mul(input.transform, float4(input.pos, 1.0f)).xyz;
    output.size = input.size;
    return output;
}