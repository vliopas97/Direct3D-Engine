Texture2D shadowMap : register(t3);
SamplerComparisonState shadowSamplerState : register(s3);

static const float shadowBias = 0.00005f;
#define SAMPLING_RADIUS 2

float Shadow(in float4 shadowPos)
{
    float shadowIntensity = 0.0f;
    
    shadowPos.xyz = shadowPos.xyz / shadowPos.w;
    
    if (shadowPos.z > 1.0f || shadowPos.z < 0.0f)
    {
        shadowIntensity = 1.0f;
    }
    else
    {
        [unroll]
        for (int x = -SAMPLING_RADIUS; x <= SAMPLING_RADIUS; x++)
        {
            [unroll]
            for (int y = -SAMPLING_RADIUS; y <= SAMPLING_RADIUS; y++)
            {
                shadowIntensity += shadowMap.SampleCmpLevelZero(shadowSamplerState, 
                shadowPos.xy, shadowPos.z - shadowBias, int2(x, y));
            }
        }
        
        shadowIntensity /= ((SAMPLING_RADIUS * 2 + 1) * (SAMPLING_RADIUS * 2 + 1));
    }
    
    return shadowIntensity;
}