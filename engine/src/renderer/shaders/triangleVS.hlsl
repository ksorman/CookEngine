
struct VSInput {
    [[vk::location(0)]] float2 position : POSITION0;
    [[vk::location(1)]] float3 color : COLOR0;
};

struct VSOut {
    float4 position : SV_Position;
    [[vk::location(0)]] float3 color : COLOR0;
};

cbuffer ubo : register(b0) {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
}

VSOut main(VSInput input)
{
    VSOut outValue;
    outValue.position = mul(proj, mul( view, mul(model, float4(input.position, 0.0f, 1.0f))));
    outValue.color = input.color;

    return outValue;
}