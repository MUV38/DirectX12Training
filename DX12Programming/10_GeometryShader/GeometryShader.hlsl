struct GSInput
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
	float2 UV : TEXCOORD0;
	float3 Normal : TEXCOORD1;
};

struct GSOutput
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
	float2 UV : TEXCOORD0;
	float3 Normal : TEXCOORD1;
};

cbuffer ShaderParameter : register(b0)
{
	float4x4 world;
	float4x4 view;
	float4x4 proj;
}

[maxvertexcount(12)]
void main(
	triangle GSInput input[3],
	inout TriangleStream< GSOutput > output
)
{
	uint i = 0;
	float4x4 mtxWorld = world;
	float4x4 mtxVP = mul(world, mul(view, proj));

	GSOutput vertex[3];
	float4 centerPos = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 centerColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float2 centerUV = float2(0.0f, 0.0f);
	float3 centerNormal = float3(0.0f, 0.0f, 0.0f);
	[unroll]
	for (i = 0; i < 3; i++)
	{
		vertex[i].Position = mul(input[i].Position, mtxWorld);
		vertex[i].Color = float4(1.0f, 1.0f, 1.0f, 1.0f);
		vertex[i].UV = input[i].UV;
		vertex[i].Normal = normalize(mul(input[i].Normal, (float3x3)mtxWorld));

		centerPos.rgb += vertex[i].Position.rgb;
		centerColor += input[i].Color;
		centerUV += input[i].UV;
		centerNormal += vertex[i].Normal;
	}
	centerPos.rgb /= 3.0f;
	centerColor /= 3.0f;
	centerUV /= 3.0f;
	centerNormal = normalize(centerNormal / 3.0f);

	const float height = 5.0f; 
	GSOutput cvertex;
	cvertex.Position = centerPos + float4(centerNormal * height, 0.0f);
	cvertex.Color = float4(0.5f, 0.5f, 0.5f, 1.0f);
	cvertex.UV = centerUV;
	cvertex.Normal = centerNormal;
	
	[unroll]
	for (i = 0; i < 3; i++)
	{
		vertex[i].Position = mul(vertex[i].Position, mtxVP);
	}
	cvertex.Position = mul(cvertex.Position, mtxVP);

	output.Append(vertex[0]);
	output.Append(vertex[1]);
	output.Append(vertex[2]);
	output.RestartStrip();

	output.Append(vertex[0]);
	output.Append(vertex[1]);
	output.Append(cvertex);
	output.RestartStrip();

	output.Append(vertex[1]);
	output.Append(vertex[2]);
	output.Append(cvertex);
	output.RestartStrip();

	output.Append(vertex[2]);
	output.Append(vertex[0]);
	output.Append(cvertex);
	output.RestartStrip();
}