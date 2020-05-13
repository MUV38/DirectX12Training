struct PSInput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 UV : TEXCOORD0;
};

cbuffer SceneParameter : register(b0)
{
    float4 Time;
}

cbuffer ShaderParameter : register(b1)
{
	float4x4 World;
	float4x4 View;
	float4x4 Proj;

	float4x4 InvView;
}

Texture2D tex : register(t0);
SamplerState samp : register(s0);

float4 main(PSInput In) : SV_TARGET
{
	return In.Color * tex.Sample(samp, In.UV);
}