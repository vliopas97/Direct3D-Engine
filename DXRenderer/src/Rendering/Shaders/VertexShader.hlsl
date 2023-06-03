struct Out
{
    float4 pos : SV_Position;
    float3 color : Color;
};

cbuffer constBuffer
{
    row_major matrix model;
};

float4 main( float3 pos : Position, float4 color : Color) : SV_Position
{
    return mul(float4(pos, 1.0f), model);
}