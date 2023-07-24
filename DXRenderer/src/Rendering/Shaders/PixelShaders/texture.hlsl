Texture2D tex : register(t0);
SamplerState samplr : register(s0);

float4 main(float3 pos : Position, float2 tc : TexCoord) : SV_TARGET
{
    return tex.Sample(samplr, tc);
}