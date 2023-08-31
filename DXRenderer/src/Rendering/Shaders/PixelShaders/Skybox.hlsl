TextureCube tex : register(t0);
SamplerState samplerTex : register(s0);

float4 main(float3 posWorld : Position) : SV_Target
{
    return tex.Sample(samplerTex, posWorld);
}