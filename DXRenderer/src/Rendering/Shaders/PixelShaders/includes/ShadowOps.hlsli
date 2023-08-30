Texture2D shadowMap : register(t3);
SamplerState shadowSamplerState : register(s3);

static const float shadowBias = 0.01f;

bool ShadowCheck(in float4 shadowPos)
{
    shadowPos.xyz = shadowPos.xyz / shadowPos.w;
    return shadowPos.z > 1.0f ? true : shadowMap.Sample(shadowSamplerState, shadowPos.xy).r > shadowPos.z - shadowBias;
}