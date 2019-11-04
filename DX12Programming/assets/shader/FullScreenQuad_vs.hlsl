struct VSInput
{
	float4 pos : POSITION;
	float2 uv  : TEXCOORD;
};

struct VSOutput
{
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;
	output.pos = input.pos;
	output.uv = input.uv;

	return output;
}