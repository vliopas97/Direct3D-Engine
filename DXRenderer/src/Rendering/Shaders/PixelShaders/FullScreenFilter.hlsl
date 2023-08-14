Texture2D tex;
SamplerState samplerTex;

float4 main(float2 position : Position) : SV_Target
{
    return 1.0f - tex.Sample(samplerTex, position).rgba;
}