//Texture2D albedoMap : register(t0);
//Texture2D normalMap : register(t1);
//Texture2D roughnessMap : register(t2);
//Texture2D metallicMap : register(t3);
//Texture2D aoMap : register(t4);
//
//Texture2D shadowDepthMap : register(t5);
//SamplerState samplerState : register(s0);

//cbuffer PixelConstantBuffer : register(b0)
//{
//    float4 ambientLighting;
//    float4 fogColor;
//    float4 data; // x = hasAO 
//};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    //float2 TextureUV : TEXCOORD0;
};


float4 main(PS_INPUT input) : SV_TARGET
{
    //float4 albedo = albedoMap.Sample(samplerState, input.TextureUV);
    return float4(1.0,1.0,1.0, 1.0);
    //return float4(albedo.xyz, 1.0f);
}