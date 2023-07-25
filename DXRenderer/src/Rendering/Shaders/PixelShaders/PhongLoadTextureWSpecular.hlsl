#include "includes\LightSource.hlsli"
#include "includes\LightVector.hlsli"
#include "includes\ShaderOps.hlsli"

cbuffer constBuffer : register(b2)
{
    row_major matrix view;
}

Texture2D tex : register(t0);
Texture2D spec : register(t1);
SamplerState samplerState : register(s0);
SamplerState samplerStateSpec : register(s1);

float4 main(float3 posCamera : Position, float3 n : Normal, float2 texCoords : TexCoords) : SV_Target
{
    n = normalize(n);
    float3 lightWorld = (float3) mul(float4(-lightPos.xy, lightPos.z, 1.0f), view);
    LightVector light = LightVectorBuild(lightWorld, posCamera);
    
    float att = Attenuation(attConst, attLin, attQuad, light.Distance);
    const float3 diffuse = Diffuse(diffuseColor, diffuseIntensity, att, light.DirectionN, n);
    
    const float4 specSample = spec.Sample(samplerStateSpec, texCoords);
    const float3 specColor = specSample.rgb;
    const float specPower = pow(2.0f, specSample.a * 13.0f);
    
    const float3 specular = Specular(specColor, 1.0f, n, light.Direction, posCamera, att, specPower);
    
    return float4(saturate(diffuse + ambient) * tex.Sample(samplerState, texCoords).rgb + specular * specColor, 1.0f);
}
