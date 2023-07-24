cbuffer constBuffer : register(b0)
{
    float3 lightPos;
    float3 ambient;
    float3 diffuseColor;
    float diffuseIntensity;
    float attConst;
    float attLin;
    float attQuad;
};

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

Texture2D tex : register(t0);
Texture2D normalMap : register(t1);

SamplerState samplerState : register(s0);
SamplerState samplerStateNormal : register(s1);

float4 main(float3 posCamera : Position, float3 n : Normal, float2 texCoords : TexCoords) : SV_Target
{
    if (NormalMapEnabled)
    {
        const float3 normalSample = normalMap.Sample(samplerStateNormal, texCoords).xyz;
        n.x = normalSample.x * 2.0f - 1.0f;
        n.y = -normalSample.y * 2.0f + 1.0f;
        n.z = -normalSample.z;
        n = mul(n, (float3x3) view);
    }
    else
        n = normalize(n);
    
    float3 lightWorld = (float3) mul(float4(-lightPos.xy, lightPos.z, 1.0f), view);
    const float3 lightDir = lightWorld- posCamera;
    const float distance = length(lightDir);
    const float3 direction = normalize(lightDir);

    const float att = 1.0f / (attConst + attLin * distance + attQuad * (distance * distance));
    const float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f, dot(direction, n));

    float3 viewDir = normalize(posCamera); // Compute view direction

    const float3 w = n * dot((lightDir), n);
    const float3 r = 2.0f * w - (lightDir);
    const float3 specular = att * (diffuseColor * diffuseIntensity) * specularIntensity * pow(max(0.0f, dot(normalize(-r), normalize(viewDir))), Shininess);

    return float4(saturate((diffuse + ambient) * tex.Sample(samplerState, texCoords).rgb + specular), 1.0f);
}