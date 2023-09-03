cbuffer constBuffer : register(b3)
{
    row_major matrix shadowView;
}

float4 ShadowConversion(const in float3 pos, uniform matrix model)
{
    const float4 posWorld = mul(float4(pos, 1.0f), model);
    return mul(posWorld, shadowView);
}