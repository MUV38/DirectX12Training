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
	float4 Time; // r:elapsedTime g:deltaTime gb:N/A
}

cbuffer GrassParameter : register(b1)
{
	float4 GrassBottomColor; // rgb:color a:N/A
	float4 GrassTopColor; // rgb:color a:N/A
	float4 GrassHeightParam; // r:height g:bottomRate b:middleRate a:topRate
	float4 GrassWidthParam; // r:width g:bottomRate b:middleRate a:topRate
	float4 GrassWindParam; // r:power g:frequency b:N/A a:N/A
	float4 GrassWindParam2; // r:powerRateBottom g:powerRateMiddle b:powerRateTop a:N/A
	float4 GrassBend; // r:bottom g:middle b:top
};

[maxvertexcount(7)]
void main(
	triangle GSInput input[3],
	inout TriangleStream< GSOutput > output
)
{
	int i = 0;

	float4x4 mtxWVP = mul(mul(world, view), proj);
	float elapsedTime = Time.x;
	float deltaTime = Time.y;

	float4 bottomColor = float4(GrassBottomColor.rgb, 1.0f);
	float4 topColor = float4(GrassTopColor.rgb, 1.0f);
	float height = GrassHeightParam.x;
	float width = GrassWidthParam.x;

	float windPower = GrassWindParam.x;
	float windFrequency = GrassWindParam.y;
	float bottomWindPowerRate = GrassWindParam2.x;
	float middleWindPowerRate= GrassWindParam2.y;
	float topWindPowerRate = GrassWindParam2.z;

	float bottomBend = GrassBend.x;
	float middleBend = GrassBend.y;
	float topBend = GrassBend.z;

	float3 p0 = input[0].Position.xyz;
	float3 p1 = input[1].Position.xyz;
	float3 p2 = input[2].Position.xyz;

	float3 n0 = input[0].Normal;
	float3 n1 = input[1].Normal;
	float3 n2 = input[2].Normal;

	float3 centerPos = (p0 + p1 + p2) / 3.0f;
	float3 centerNormal = (n0 + n1 + n2) / 3.0f;
	float centerHeight = 1.0f;
	float centerRot = 0.0f;
	float centerWind = 1.0f;

	float bottomHeight = centerHeight * height * GrassHeightParam.y;
	float middleHeight = centerHeight * height * GrassHeightParam.z;
	float topHeight = centerHeight * height * GrassHeightParam.w;

	float bottomWidth = width * GrassWidthParam.y;
	float middleWidth = width * GrassWidthParam.z;
	float topWidth = width * GrassWidthParam.w;

	centerRot = centerRot - 0.5f;
	float3 dir = normalize((p2 - p0) * centerRot);

	GSOutput gsOut[7];
	[unroll]
	for (i = 0; i < 7; ++i)
	{
		gsOut[i] = (GSOutput)0;
	}

	// bottom
	gsOut[0].Position = float4(centerPos - dir * bottomWidth, 1.0f);
	gsOut[0].Color = bottomColor;

	gsOut[1].Position = float4(centerPos + dir * bottomWidth, 1.0f);
	gsOut[1].Color = bottomColor;

	// bottom to middle
	gsOut[2].Position = float4(centerPos - dir * middleWidth + centerNormal * bottomHeight, 1.0f);
	gsOut[2].Color = lerp(bottomColor, topColor, 0.33333f);

	gsOut[3].Position = float4(centerPos + dir * middleWidth + centerNormal * bottomHeight, 1.0f);
	gsOut[3].Color = lerp(bottomColor, topColor, 0.33333f);

	// middle to top
	gsOut[4].Position = float4(gsOut[3].Position.xyz - dir * topWidth + centerNormal * middleHeight, 1.0f);
	gsOut[4].Color = lerp(bottomColor, topColor, 0.666666f);

	gsOut[5].Position = float4(gsOut[3].Position.xyz + dir * topWidth + centerNormal * middleHeight, 1.0f);
	gsOut[5].Color = lerp(bottomColor, topColor, 0.666666f);

	// top
	gsOut[6].Position = float4(gsOut[5].Position.xyz + dir * topWidth + centerNormal * topHeight, 1.0f);
	gsOut[6].Color = topColor;

	// Bend
	dir = float3(1.0f, 0.0f, 0.0f);
	gsOut[2].Position.xyz += dir * (windPower * centerWind * bottomBend) * sin(elapsedTime);
	gsOut[3].Position.xyz += dir * (windPower * centerWind * bottomBend) * sin(elapsedTime);
	gsOut[4].Position.xyz += dir * (windPower * centerWind * middleBend) * sin(elapsedTime);
	gsOut[5].Position.xyz += dir * (windPower * centerWind * middleBend) * sin(elapsedTime);
	gsOut[6].Position.xyz += dir * (windPower * centerWind * topBend) * sin(elapsedTime);

	[unroll]
	for (i = 0; i < 7; i++)
	{
		gsOut[i].Position = mul(gsOut[i].Position, mtxWVP);
		output.Append(gsOut[i]);
	}
}