struct VSInput
{
    float3 Position : POSITION;
    float4 Color : COLOR;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
};
struct VSOutput
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

VSOutput main(VSInput In)
{
    VSOutput result = (VSOutput)0;
    result.Position = float4(In.Position, 1.0f);
    result.Color = In.Color;
    result.UV = In.UV;
    result.Normal = In.Normal;
    return result;
}