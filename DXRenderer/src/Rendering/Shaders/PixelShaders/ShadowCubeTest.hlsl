#include "includes\LightSource.hlsli"
#include "includes\ShadowOps.hlsli"

cbuffer constBuffer : register(b1)
{
    float4 materialColor;
};

float4 main(float3 posCamera : Position, float3 n : Normal, float4 shadowPos : ShadowPosition) : SV_Target
{
    float4 color = float4(materialColor);
    
    if (!ShadowCheck(shadowPos))
    {
        color = float4(ambient, 1.0f) * materialColor;
    }

    return color;
}