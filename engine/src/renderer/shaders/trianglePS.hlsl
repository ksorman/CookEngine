struct PSInput {
    float4 Position : SV_Position;
    //float3 Color : COLOR;
};

struct PSOut {
    float4 Color;
};

float4 main(PSInput input) : SV_Target
{
    return float4(input.Position.xyz, 1.0f);
}