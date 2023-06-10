cbuffer constBuffer : register(b0)
{
    row_major matrix projection;
}

cbuffer constBuffer2 : register(b1)
{
    row_major matrix model;
};

float4 main( float3 pos : Position) : SV_Position
{
    return mul(mul(float4(pos, 1.0f), model), projection);
}