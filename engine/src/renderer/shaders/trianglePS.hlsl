struct PSInput {
    float4 position : SV_Position;
    [[vk::location(0)]] float3 color : COLOR0;
    [[vk::location(1)]] float2 tc : TEXCOORD0;
};

struct PSOut {
    float4 color;
};

SamplerState texSampler : register(s1);
Texture2D<float4> textureAlbedo : register(t1);

float4 main(PSInput input) : SV_Target
{
    return textureAlbedo.Sample(texSampler, input.tc);
}