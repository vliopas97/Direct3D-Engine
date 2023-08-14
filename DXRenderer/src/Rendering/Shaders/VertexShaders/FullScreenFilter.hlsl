struct Output
{
    float2 uv : TexCoord;
    float4 pos : SV_Position;
};

Output main(float2 position : Position)
{
    Output output;
    output.pos = float4(position, 0.0f, 1.0f);
    output.uv = float2((position.x + 1) / 2.0f, -(position.y - 1) / 2.0f);
    return output;
}