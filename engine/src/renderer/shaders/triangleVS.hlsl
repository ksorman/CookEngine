struct VSInput {

    int Index : SV_VertexID;
};

struct VSOut {
    float4 Position : SV_Position;
    //float3 Color : COLOR;
};

static float3 positions[3] = {
    {0.0, -0.5, 0.0f},
    {0.5, 0.5, 0.0f},
    {-0.5, 0.5, 0.0f}
};

VSOut main(VSInput input)
{
    VSOut outValue;
    outValue.Position = float4(positions[input.Index], 1.0f);
    //outValue.Color = input.Color;

    return outValue;
}