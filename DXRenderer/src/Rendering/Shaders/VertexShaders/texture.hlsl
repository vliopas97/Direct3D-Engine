cbuffer constBuffer : register(b0)
{
    row_major matrix projection;
}

cbuffer constBuffer2 : register(b1)
{
    row_major matrix model;
};

struct Out
{
    float4 pos : SV_Position;
    float2 tex : TexCoord;
};

Out main( float3 pos : Position, float2 tex : TexCoord )
{
    Out output;
    output.pos = mul(mul(float4(pos, 1.0f), model), projection);
    output.tex = tex;
	return output;
}