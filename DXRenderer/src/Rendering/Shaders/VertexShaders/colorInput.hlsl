struct Out
{
    float4 pos : SV_Position;
    float3 color : Color;
};

cbuffer constBuffer0 : register(b0)
{
    row_major matrix projection;
}

cbuffer constBuffer2 : register(b1)
{
    row_major matrix model;
};

Out main(float3 pos : Position, float4 col : Color)
{
    Out output;
    output.pos = mul(mul(float4(pos, 1.0f), model), projection);
    output.color = col;
    return output;
}