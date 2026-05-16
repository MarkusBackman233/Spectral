cbuffer ViewConstantBuffer : register(b0)
{
    matrix ViewProjection;
    float4 CameraPosition;
    float CB_elapsedTime;
    float CB_unused1;
    float CB_unused2;
    float CB_unused3;
};

struct GSInput
{
    float3 worldpos : POSITION;
    nointerpolation float size : SIZE;
};

struct GSOutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

[maxvertexcount(4)]
void main(point GSInput input[1], inout TriangleStream<GSOutput> TriStream)
{
    GSOutput output;

    float3 center = input[0].worldpos;

    float width = input[0].size;
    float height = input[0].size;

    // Camera-facing billboard basis
    float3 forward = normalize(CameraPosition.xyz - center);

    float3 up = float3(0.0f, 1.0f, 0.0f);

    float3 right = normalize(cross(up, forward));
    up = normalize(cross(forward, right));

    float hw = width * 0.5f;
    float hh = height * 0.5f;

    // --------------------------------------------------
    // WIND
    // --------------------------------------------------

    float time = CB_elapsedTime;

    float windStrength = 0.01f;
    float windSpeed = 7.0f;

    // Per-instance variation
    float phase =
        center.x * 0.5f +
        center.z * 0.35f;

    // Layered waves for more natural motion
    float wind1 =
        sin(time * windSpeed + phase);

    float wind2 =
        sin(time * 0.7f + phase * 2.1f);

    float wind =
        wind1 * 0.7f +
        wind2 * 0.3f;

    // Wind direction
    float3 windDir =
        normalize(float3(1.0f, 0.0f, 0.4f));

    float bend =
        wind * windStrength;

    // --------------------------------------------------
    // BUILD QUAD
    // --------------------------------------------------

    float3 corners[4];

    // Top-left
    corners[0] =
        center +
        (-right * hw) +
        (up * hh);

    // Top-right
    corners[1] =
        center +
        (right * hw) +
        (up * hh);

    // Bottom-left
    corners[2] =
        center +
        (-right * hw) +
        (-up * hh);

    // Bottom-right
    corners[3] =
        center +
        (right * hw) +
        (-up * hh);

    // --------------------------------------------------
    // APPLY WIND BENDING
    // Only top vertices move
    // --------------------------------------------------

    corners[0] += windDir * bend;
    corners[1] += windDir * bend;
    corners[2] += windDir * bend*0.5;
    corners[3] += windDir * bend*0.5;

    // --------------------------------------------------
    // UVS
    // --------------------------------------------------

    float2 uvs[4] =
    {
        float2(0.0f, 0.0f),
        float2(1.0f, 0.0f),
        float2(0.0f, 1.0f),
        float2(1.0f, 1.0f)
    };

    // --------------------------------------------------
    // OUTPUT
    // --------------------------------------------------

    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        output.pos =
            mul(float4(corners[i], 1.0f), ViewProjection);

        output.uv = uvs[i];

        TriStream.Append(output);
    }

    TriStream.RestartStrip();
}