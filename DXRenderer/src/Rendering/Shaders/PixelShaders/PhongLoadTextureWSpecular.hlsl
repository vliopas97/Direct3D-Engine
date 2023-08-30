#include "includes\LightSource.hlsli"
#include "includes\LightVector.hlsli"
#include "includes\ShaderOps.hlsli"
#include "includes\ShadowOps.hlsli"

cbuffer constBuffer : register(b2)
{
    row_major matrix view;
}

Texture2D tex : register(t0);
Texture2D spec : register(t2);
SamplerState samplerState : register(s0);
SamplerState samplerStateSpec : register(s2);

float4 main(float3 posCamera : Position, float3 n : Normal, float2 texCoords : TexCoords, float4 shadowPos : ShadowPosition) : SV_Target
{
    float3 diffuse = float3(0.0f, 0.0f, 0.0f);
    float3 specular = float3(0.0f, 0.0f, 0.0f);
    
    const float shadowIntensity = Shadow(shadowPos);
    if (shadowIntensity != 0.0f)
    {
        n = normalize(n);
        float3 lightWorld = (float3) mul(float4(-lightPos.xy, lightPos.z, 1.0f), view);
        LightVector light = LightVectorBuild(lightWorld, posCamera);
    
        float att = Attenuation(attConst, attLin, attQuad, light.Distance);
        diffuse = Diffuse(diffuseColor, diffuseIntensity, att, light.DirectionN, n);
    
        const float4 specSample = spec.Sample(samplerStateSpec, texCoords);
        const float3 specColor = specSample.rgb;
        const float specPower = pow(2.0f, specSample.a * 13.0f);
    
        specular = Specular(specColor, 1.0f, n, light.Direction, posCamera, att, specPower);
        
        diffuse *= shadowIntensity;
        specular *= shadowIntensity;
    }
    
    return float4(saturate(diffuse + ambient) * tex.Sample(samplerState, texCoords).rgb + specular, 1.0f);
}
