struct LightVector
{
    float3 Direction;
    float Distance;
    float3 DirectionN;
};

LightVector LightVectorBuild(const in float3 lightPos, const in float3 objectPos)
{
    LightVector light;
    
    light.Direction = lightPos - objectPos;
    light.Distance = length(light.Direction);
    light.DirectionN = normalize(light.Direction);
    
    return light;
}