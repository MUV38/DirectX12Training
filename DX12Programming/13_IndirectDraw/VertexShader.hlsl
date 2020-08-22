struct VSInput
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float4 Color    : COLOR;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float3 Normal   : NORMAL;
    float4 Color    : COLOR;
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
    result.Normal = normalize(mul(In.Normal, (float3x3)world));
    result.Color = In.Color;
    
    return result;
}