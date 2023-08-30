cbuffer constBuffer : register(b0)
{
    row_major matrix model;
}

cbuffer constBuffer : register(b3)
{
    row_major matrix shadowViewProj;
}

float4 main( float3 pos : Position ) : SV_Position
{
    return mul(mul(float4(pos, 1.0f), model), shadowViewProj);
}