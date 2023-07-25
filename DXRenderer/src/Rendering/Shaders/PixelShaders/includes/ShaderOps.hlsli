float Attenuation(uniform float attConst, uniform float attLin, uniform float attQuad, const in float distance)
{
    return 1.0f / (attConst + attLin * distance + attQuad * (distance * distance));
}

float3 Diffuse(
    uniform float3 diffuseColor,
    uniform float diffuseIntensity,
    const in float att,
    const in float3 lightDir,
    const in float3 normal)
{
    return diffuseColor * diffuseIntensity * att * max(0.0f, dot(lightDir, normal));
}

float3 normalPreprocessing(const in float3 normal,
    const in float3 tangent,
    const in float3 bitangent,
    float2 texCoords,
    uniform Texture2D normalMap,
    uniform SamplerState samplerStateNormal)
{
    float3 n;
    float3x3 TBN = float3x3(normalize(tangent), normalize(bitangent), normalize(normal));
    const float3 normalSample = normalMap.Sample(samplerStateNormal, texCoords).xyz;
    n = normalSample * 2.0f - 1.0f;
    n = mul(n, TBN);
    return normalize(n);
}

float3 Specular(
    const in float3 specularColor,
    uniform float specularIntensity,
    const in float3 n,
    const in float3 lightDir,
    const in float3 posCamera,
    const in float att,
    const in float specularPower)
{
    const float3 w = n * dot(lightDir, n);
    const float3 r = normalize(w * 2.0f - lightDir);
    const float3 viewDir = normalize(posCamera);

    return att * specularColor * specularIntensity * pow(max(0.0f, dot(-r, viewDir)), specularPower);
}