cbuffer constBuffer : register(b0)
{
    row_major matrix viewProj;
}

struct Output
{
    float3 posWorld : Position;
    float4 pos : SV_Position;
};

Output main(float3 pos : Position)
{
    Output output;
    output.posWorld = pos;
    output.pos = mul(float4(pos, 0.0f), viewProj);
    output.pos.z = output.pos.w;
    return output;
}