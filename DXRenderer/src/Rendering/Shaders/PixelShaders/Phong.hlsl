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
};

cbuffer constBuffer : register(b2)
{
    row_major matrix view;
}

float4 main(float3 posWorld : Position, float3 normal : Normal) : SV_Target
{
    float3 n = normalize(normal);
    float3 lightWorld = (float3) mul(float4(-lightPos.xy, lightPos.z, 1.0f), view);
    
    float3 lightDir = lightWorld - posWorld;
    const float distance = length(lightDir);
    const float3 direction = normalize(lightDir);

    const float att = 1.0f / (attConst + attLin * distance + attQuad * (distance * distance));
    const float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f, dot(direction, n));

    float3 viewDir = normalize(posWorld); // Compute view direction

    const float3 w = n * dot(lightDir, n);
    const float3 r = 2.0f * w - (lightDir);
    const float3 specular = att * (diffuseColor * diffuseIntensity) * specularIntensity * pow(max(0.0f, dot(normalize(-r), normalize(viewDir))), Shininess);

    return float4(saturate((diffuse + ambient + specular) * materialColor), 1.0f);
}
