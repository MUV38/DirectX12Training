RWTexture2D<float4> InOutTexture : register(u0);

#define THREAD_X 16
#define THREAD_Y 16

[numthreads(THREAD_X, THREAD_Y, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint2 textureIndex = DTid.xy;

	float4 color = InOutTexture[textureIndex];
	color.rgb = dot(color.rgb, float3(0.2126f, 0.7152f, 0.0722f));
	InOutTexture[textureIndex] = color;
}