
cbuffer constBuffer2 : register(b0)
{
    row_major matrix modelView;
};

cbuffer constBuffer0 : register(b1)
{
    row_major matrix projection;
}


float4 main(float3 pos : Position) : SV_Position
{
    return mul(mul(float4(pos, 1.0f), modelView), projection);;
}