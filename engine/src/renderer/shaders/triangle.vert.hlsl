
struct VSInput {
    [[vk::location(0)]] float3 position : POSITION0;
    [[vk::location(1)]] float2 tc : TEXCOORD0;
};

struct VSOut {
    float4 position : SV_Position;
    [[vk::location(0)]] float2 tc : TEXCOORD0;
};

cbuffer ubo : register(b0) {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
}

VSOut main(VSInput input)
{
    VSOut outValue;
    outValue.position = mul(proj, mul( view, mul(model, float4(input.position, 1.0f))));
    outValue.tc = input.tc;

    return outValue;
}