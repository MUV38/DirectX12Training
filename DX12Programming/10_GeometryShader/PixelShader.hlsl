struct PSInput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 UV : TEXCOORD0;
    float3 Normal : TEXCOORD1;
};

float4 main(PSInput In) : SV_TARGET
{
    return In.Color;
}