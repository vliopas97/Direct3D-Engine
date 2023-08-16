Texture2D tex;
SamplerState samplerTex;

static const int r = 7;
static const float dim = (2 * r + 1) * (2 * r + 1);

float4 main(float2 position : Position) : SV_Target
{
    uint width, height;
    tex.GetDimensions(width, height);
    const float dx = 1.0f / width;
    const float dy = 1.0f / height;
    float accAlpha = 0.0f;
    float3 maxColor = float3(0.0f, 0.0f, 0.0f);
    for (int y = -r; y <= r; y++)
    {
        for (int x = -r; x <= r; x++)
        {
            const float2 tc = position + float2(dx * x, dy * y);
            const float4 s = tex.Sample(samplerTex, tc).rgba;
            accAlpha += s.a;
            maxColor = max(s.rgb, maxColor);
        }
    }
    return float4(maxColor, accAlpha / dim);
}