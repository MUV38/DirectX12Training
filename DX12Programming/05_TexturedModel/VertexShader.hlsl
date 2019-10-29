struct VSInput
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 UV       : TEXCOORD;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 UV       : TEXCOORD0;
    float3 Normal   : TEXCOORD1;
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
    float4x4 mtxWVP = mul(world, mul(view, proj));
    result.Position = mul(float4(In.Position, 1.0f), mtxWVP);
    result.UV = In.UV;
    result.Normal = In.Normal;
    return result;
}