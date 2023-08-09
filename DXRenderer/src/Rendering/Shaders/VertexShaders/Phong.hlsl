cbuffer constBuffer : register(b0)
{
    row_major matrix modelView;
}

cbuffer constBuffer : register(b1)
{
    row_major matrix projection;
}

struct Output
{
    float3 posWorld : Position;
    float3 normal : Normal;
    float4 pos : SV_Position;
};

Output main( float3 pos : Position, float3 n : Normal )
{
    Output output;
    
    output.posWorld = (float3) mul(float4(pos, 1.0f), modelView);
    output.normal = mul(n, (float3x3) modelView);
    output.pos = mul(float4(output.posWorld, 1.0f), projection);
    return output;
}