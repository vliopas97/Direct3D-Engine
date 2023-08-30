#include "includes\LightSource.hlsli"
#include "includes\LightVector.hlsli"
#include "includes\ShaderOps.hlsli"
#include "includes\ShadowOps.hlsli"

cbuffer constBuffer : register(b1)
{
    float3 materialColor;
    float specularIntensity;
    float Shininess;
    bool NormalMapEnabled;
};

cbuffer constBuffer : register(b2)
{
    row_major matrix view;
}

Texture2D tex;
SamplerState samplerState;

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
        specular = Specular(materialColor, specularIntensity, n, light.Direction, posCamera, att, Shininess);
        
        diffuse *= shadowIntensity;
        specular *= shadowIntensity;
    }
    
    return float4(saturate((diffuse + ambient) * tex.Sample(samplerState, texCoords).rgb + specular), 1.0f);
}
