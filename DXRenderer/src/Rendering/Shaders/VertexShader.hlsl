struct Out
{
    float4 pos : SV_Position;
    float3 color : Color;
};

cbuffer constBuffer2 : register(b0)
{
    row_major matrix projection;
}

cbuffer constBuffer : register(b1)
{
    row_major matrix model;
};


float4 main( float3 pos : Position, float4 color : Color) : SV_Position
{
    return mul(mul(float4(pos, 1.0f), model), projection);
}