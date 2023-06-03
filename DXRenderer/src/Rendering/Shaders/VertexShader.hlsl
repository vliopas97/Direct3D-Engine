struct Out
{
    float4 pos : SV_Position;
    float3 color : Color;
};

Out main( float2 pos : Position, float4 color : Color)
{
    Out output;
	output.pos = float4(pos.x,pos.y,0.0f,1.0f);
    output.color = float3(color.x, color.y, color.z);
    return output;
}