// Input control point
struct VS_CONTROL_POINT_OUTPUT
{
	float3 Position : WORLDPOS;
	float4 Color : COLOR;
};

// Output control point
struct HS_CONTROL_POINT_OUTPUT
{
	float3 Position : WORLDPOS;
	float4 Color : COLOR;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]			: SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTessFactor			: SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
};

// ShaderParameter
cbuffer TessellationParameters : register(b0)
{
	float4 TessFactor;
	float4 TessFactor2;
};

#define NUM_CONTROL_POINTS 3

// Patch Constant Function
HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;

	// Insert code to compute Output here
	float subdivision = TessFactor2.x;
	Output.EdgeTessFactor[0] = TessFactor.x * subdivision;
	Output.EdgeTessFactor[1] = TessFactor.y * subdivision;
	Output.EdgeTessFactor[2] = TessFactor.z * subdivision;
	Output.InsideTessFactor = TessFactor.w * subdivision;

	return Output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_CONTROL_POINT_OUTPUT main( 
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip, 
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
	HS_CONTROL_POINT_OUTPUT Output;

	// Insert code to compute Output here
	Output.Position = ip[i].Position;
	Output.Color = ip[i].Color;

	return Output;
}
