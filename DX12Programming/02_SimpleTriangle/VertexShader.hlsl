struct VSInput
{
    float4 Position : POSITION;
    float4 Color : COLOR;
};
struct VSOutput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

VSOutput main(VSInput In)
{
    VSOutput output = (VSOutput)0;

    output.Position = In.Position;
    output.Color = In.Color;

	return output;
}