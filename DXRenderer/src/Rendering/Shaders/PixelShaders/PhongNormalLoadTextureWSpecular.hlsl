#include "includes\LightSource.hlsli"
#include "includes\LightVector.hlsli"
#include "includes\ShaderOps.hlsli"
#include "includes\ShadowOps.hlsli"

cbuffer constBuffer : register(b2)
{
    row_major matrix view;
}

Texture2D tex : register(t0);
Texture2D normalMap : register(t1);
Texture2D spec : register(t2);
SamplerState samplerState : register(s0);
SamplerState samplerStateNormal : register(s1);
SamplerState samplerStateSpec : register(s2);


float4 main(float3 posCamera : Position, float3 n : Normal, float3 t : Tangent, float3 b : Bitangent, float2 texCoords : TexCoords, float4 shadowPos : ShadowPosition) : SV_Target
{
    float3 diffuse = float3(0.0f, 0.0f, 0.0f);
    float3 specular = float3(0.0f, 0.0f, 0.0f);
    float4 texSample = tex.Sample(samplerState, texCoords);
    
    clip(texSample.a < 0.1f ? -1 : 1);
    
    if (ShadowCheck(shadowPos))
    {
        if (dot(n, posCamera) >= 0)
            n = -n;
    
        n = normalPreprocessing(n, t, b, texCoords, normalMap, samplerStateNormal);
    
        float3 lightWorld = (float3) mul(float4(-lightPos.xy, lightPos.z, 1.0f), view);
        LightVector light = LightVectorBuild(lightWorld, posCamera);
    
        float att = Attenuation(attConst, attLin, attQuad, light.Distance);
        diffuse = Diffuse(diffuseColor, diffuseIntensity, att, light.DirectionN, n);
    
        const float4 specSample = spec.Sample(samplerStateSpec, texCoords);
        const float3 specColor = specSample.rgb;
        const float specPower = pow(2.0f, specSample.a * 13.0f);

        specular = Specular(specColor, 1.0f, n, light.Direction, posCamera, att, specPower);
    }
    
    return float4(saturate(diffuse + ambient) * texSample.rgb + specular, texSample.a);
}