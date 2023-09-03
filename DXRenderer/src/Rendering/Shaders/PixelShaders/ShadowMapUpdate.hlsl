static const float shadowBias = 0.0005f;

float4 main(float3 pos : Position) : SV_TARGET
{
    return length(pos) / 100.0f + shadowBias;
}