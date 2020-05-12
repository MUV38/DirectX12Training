struct DS_OUTPUT
{
	float4 Position  : SV_POSITION;
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
	// TODO: change/add other stuff
};

// ShaderParameter
cbuffer TessellationParameters : register(b0)
{
	float4 TessFactor;
}

#define NUM_CONTROL_POINTS 3

[domain("tri")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
	DS_OUTPUT Output;

	Output.Position = float4(
		patch[0].Position *domain.x+patch[1].Position *domain.y+patch[2].Position *domain.z,1);
	Output.Color = patch[0].Color * domain.x + patch[1].Color * domain.y + patch[2].Color * domain.z;

	return Output;
}
