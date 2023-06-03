cbuffer colors
{
    float4 face_colors[6];
};

float4 main(uint index : SV_PrimitiveID) : SV_Target
{
    return face_colors[index / 2];
}
