TextureCube shadowMap : register(t3);
SamplerComparisonState shadowSamplerState : register(s3);

static const float zf = 100.0f;
static const float zn = 0.5f;
static const float c1 = zf / (zf - zn);
static const float c0 = -zn * zf / (zf - zn);

float CalculateShadowDepth(const in float4 shadowPos)
{
    const float3 m = abs(shadowPos).xyz;
    const float major = max(m.x, max(m.y, m.z));
    return (c1 * major + c0) / major;
}

float Shadow(in float4 shadowPos)
{
    shadowPos.xy = -shadowPos.xy;
    return shadowMap.SampleCmpLevelZero(shadowSamplerState, shadowPos.xyz, CalculateShadowDepth(shadowPos));
}