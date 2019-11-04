struct PSInput
{
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

Texture2D tex : register(t0);
SamplerState samp : register(s0);

float4 main(PSInput input) : SV_TARGET
{
	float4 color = tex.Sample(samp, input.uv);
	return color;
}