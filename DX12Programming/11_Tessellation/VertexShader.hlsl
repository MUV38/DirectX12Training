struct VSInput
{
    float3 Position : POSITION;
    float4 Color : COLOR;
};
struct VSOutput
{
    float3 Position : WORLDPOS;
    float4 Color : COLOR;
};

VSOutput main(VSInput In)
{
    VSOutput output = (VSOutput)0;

    output.Position = In.Position;
    output.Color = In.Color;

	return output;
}