struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 UV       : TEXCOORD0;
    float3 Normal   : TEXCOORD1;
};

Texture2D tex : register(t0);
SamplerState samp : register(s0);

float4 main(VSOutput In) : SV_TARGET
{
	return tex.Sample(samp, In.UV);

    //float3 normal = normalize(In.Normal);
    //return float4(normal * 0.5f + 0.5f, 1.0f);
}