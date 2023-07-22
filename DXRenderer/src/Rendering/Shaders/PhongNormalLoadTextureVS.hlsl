cbuffer constBuffer : register(b0)
{
    row_major matrix view;
}

cbuffer constBuffer : register(b1)
{
    row_major matrix projection;
}

cbuffer constBuffer : register(b2)
{
    row_major matrix model;
};

struct Output
{
    float3 posCamera : Position;
    float3 normal : Normal;
    float3 tangent : Tangent;
    float3 bitangent : Bitangent;
    float2 texCoords : TexCoord;
    float4 pos : SV_Position;
};

Output main(float3 pos : Position, float3 n : Normal, float3 t : Tangent, float3 b : Bitangent, float2 texCoords : TexCoords)
{
    Output output;
    matrix modelView = mul(model, view);
    output.posCamera = (float3) mul(float4(pos, 1.0f), modelView);
    output.normal = mul(n, (float3x3) modelView);
    output.tangent = mul(t, (float3x3) modelView);
    output.bitangent = mul(b, (float3x3) modelView);
    output.texCoords = texCoords;
    output.pos = mul(float4(output.posCamera, 1.0f), projection);
    return output;
}