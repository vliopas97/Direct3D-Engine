cbuffer constBuffer
{
    float3 lightPos;
};

static const float3 materialColor = { 0.7f, 0.7f, 0.9f };
static const float3 ambient = { 0.15f, 0.15f, 0.15f };
static const float3 diffuseColor = { 1.0f, 1.0f, 1.0f };
static const float diffuseIntensity = 1.0f;
static const float attConst = 1.0f;
static const float attLin = 0.05f;
static const float attQuad = 0.01f;

float4 main(float3 posWorld : Position, float3 n : Normal) : SV_Target
{
    const float3 vToL = lightPos - posWorld;
    const float distance = length(vToL);
    const float3 direction = vToL / distance;
    
    const float att = 1.0f/(attConst + attLin * distance + attQuad * (distance * distance));
    const float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f, dot(direction, n));
    
    return float4(saturate(diffuse + ambient), 1.0f);
}