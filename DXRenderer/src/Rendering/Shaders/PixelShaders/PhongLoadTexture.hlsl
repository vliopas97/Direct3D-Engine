#include "includes\LightSource.hlsli"
#include "includes\LightVector.hlsli"
#include "includes\ShaderOps.hlsli"

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

float4 main(float3 posCamera : Position, float3 n : Normal, float2 texCoords : TexCoords) : SV_Target
{
    n = normalize(n);
    float3 lightWorld = (float3) mul(float4(-lightPos.xy, lightPos.z, 1.0f), view);
    
    LightVector light = LightVectorBuild(lightWorld, posCamera);

    float att = Attenuation(attConst, attLin, attQuad, light.Distance);
    const float3 diffuse = Diffuse(diffuseColor, diffuseIntensity, att, light.DirectionN, n);
    float3 specular = Specular(materialColor, specularIntensity, n, light.Direction, posCamera, att, Shininess);
    
    return float4(saturate((diffuse + ambient) * tex.Sample(samplerState, texCoords).rgb + specular), 1.0f);
}