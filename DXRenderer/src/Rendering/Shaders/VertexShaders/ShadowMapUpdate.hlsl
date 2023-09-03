cbuffer constBuffer : register(b0)
{
    row_major matrix model;
}

cbuffer constBuffer : register(b4)
{
    row_major matrix shadowViewProj;
}

float4 main( float3 pos : Position ) : SV_Position
{
    float4 posProj = mul(mul(float4(pos, 1.0f), model), shadowViewProj);
    posProj.xy = -posProj.xy;
    return posProj;
}