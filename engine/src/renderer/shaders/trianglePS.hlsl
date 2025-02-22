struct PSInput {
    float4 position : SV_Position;
    [[vk::location(0)]] float3 color : COLOR0;
};

struct PSOut {
    float4 color;
};

float4 main(PSInput input) : SV_Target
{
    return float4(input.color, 1.0f);
}