struct GSInput
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
};

struct GSOutput
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

static const float3 g_positions[4] =
{
	float3(-0.5f,  0.5f, 0.0f),
	float3( 0.5f,  0.5f, 0.0f),
	float3(-0.5f, -0.5f, 0.0f),
	float3( 0.5f, -0.5f, 0.0f),
};

static const float2 g_texcoords[4] =
{
	float2(0.0f, 0.0f),
	float2(1.0f, 0.0f),
	float2(0.0f, 1.0f),
	float2(1.0f, 1.0f),
};

[maxvertexcount(4)]
void main(
	point GSInput input[1],
	inout TriangleStream< GSOutput > output
)
{
	float4x4 mtxWVP = mul(World, mul(View, Proj));
	float4x4 mtxVP = mul(View, Proj);

	[unroll]
	for (uint i = 0; i < 4; i++)
	{
		float4 pos = float4(mul(g_positions[i].xyz * 0.1f, (float3x3)InvView) + input[0].Position.xyz * 0.2f, 1.0f);
		pos = mul(pos, mtxVP);

		GSOutput element;
		element.Position = pos;
		element.Color = input[0].Color;
		element.UV = g_texcoords[i];
		output.Append(element);
	}
	output.RestartStrip();
}