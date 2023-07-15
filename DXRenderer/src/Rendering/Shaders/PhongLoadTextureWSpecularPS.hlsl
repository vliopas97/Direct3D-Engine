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

cbuffer constBuffer : register(b2)
{
    row_major matrix view;
}

Texture2D tex : register(t0);
Texture2D spec : register(t1);
SamplerState samplerState : register(s0);
SamplerState samplerStateSpec : register(s1);

float4 main(float3 posWorld : Position, float3 n : Normal, float2 texCoords : TexCoords) : SV_Target
{
    n = normalize(n);
    float3 lightWorld = (float3) mul(float4(-lightPos.xy, lightPos.z, 1.0f), view);
    float3 lightDir = lightWorld - posWorld;
    const float distance = length(lightDir);
    const float3 direction = normalize(lightDir);

    const float att = 1.0f / (attConst + attLin * distance + attQuad * (distance * distance));
    const float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f, dot(direction, n));

    float3 viewDir = normalize(posWorld); // Compute view direction

    const float3 w = n * dot(lightDir, n);
    const float3 r = 2.0f * w - lightDir;
    
    const float4 specSample = spec.Sample(samplerStateSpec, texCoords);
    const float3 specColor = specSample.rgb;
    const float specPower = pow(2.0f, specSample.a * 13.0f);

    const float3 specular = att * diffuseColor * pow(max(0.0f, dot(normalize(-r), normalize(viewDir))), specPower);
    
    return float4(saturate(diffuse + ambient) * tex.Sample(samplerState, texCoords).rgb + specular * specColor, 1.0f);
}
