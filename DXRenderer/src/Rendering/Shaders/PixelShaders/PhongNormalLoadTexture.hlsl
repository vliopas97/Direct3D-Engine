#include "includes\LightSource.hlsli"
#include "includes\LightVector.hlsli"
#include "includes\ShaderOps.hlsli"
#include "includes\ShadowOps.hlsli"

cbuffer constBuffer : register(b1) // TO DO: FIX ALIGNMENT ISSUES
{
    float3 materialColor;
    float specularIntensity;
    float Shininess;
    float NormalMapEnabled;
};

cbuffer constBuffer : register(b2)
{
    row_major matrix view;
}

Texture2D tex : register(t0);
Texture2D normalMap : register(t1);
SamplerState samplerState : register(s0);
SamplerState samplerStateNormal : register(s1);


float4 main(float3 posCamera : Position, float3 n : Normal, float3 t : Tangent, float3 b : Bitangent, float2 texCoords : TexCoords, float4 shadowPos : ShadowPosition) : SV_Target
{
    float3 diffuse = float3(0.0f, 0.0f, 0.0f);
    float3 specular = float3(0.0f, 0.0f, 0.0f);
    
    if (ShadowCheck(shadowPos))
    {
        n = normalPreprocessing(n, t, b, texCoords, normalMap, samplerStateNormal);
    
        float3 lightWorld = (float3) mul(float4(-lightPos.xy, lightPos.z, 1.0f), view);
        LightVector light = LightVectorBuild(lightWorld, posCamera);

        float att = Attenuation(attConst, attLin, attQuad, light.Distance);
        diffuse = Diffuse(diffuseColor, diffuseIntensity, att, light.DirectionN, n);
        //float3 specular = Specular(materialColor, specularIntensity, n, light.Direction, posCamera, att, Shininess);
    }

    return float4(saturate((diffuse + ambient) * tex.Sample(samplerState, texCoords).rgb), 1.0f);
}
