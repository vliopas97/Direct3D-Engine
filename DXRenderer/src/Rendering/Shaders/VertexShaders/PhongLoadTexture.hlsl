cbuffer constBuffer : register(b0)
{
    row_major matrix model;
}

cbuffer constBuffer : register(b1)
{
    row_major matrix modelView;
}

cbuffer constBuffer : register(b2)
{
    row_major matrix projection;
}

cbuffer constBuffer : register(b3)
{
    row_major matrix shadowViewProj;
}

struct Output
{
    float3 posCamera : Position;
    float3 normal : Normal;
    float2 texCoords : TexCoords;
    float4 shadowPos : ShadowPosition;
    float4 pos : SV_Position;
};

Output main(float3 pos : Position, float3 n : Normal, float2 texCoords : TexCoords)
{
    Output output;
    
    output.posCamera = (float3) mul(float4(pos, 1.0f), modelView);
    output.normal = mul(n, (float3x3) modelView);
    output.texCoords = texCoords;
    output.pos = mul(float4(output.posCamera, 1.0f), projection);
    
    const float4 shadowProj = mul(mul(float4(pos, 1.0f), model), shadowViewProj);
    output.shadowPos = shadowProj * float4(0.5f, -0.5f, 1.0f, 1.0f) + (float4(0.5f, 0.5f, 0.0f, 0.0f) * shadowProj.w);
    
    return output;
}