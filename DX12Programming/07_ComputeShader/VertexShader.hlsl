struct VSInput
{
    uint VertexID : SV_VertexID;
    float4 Color : COLOR;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

cbuffer SceneParameter : register(b0)
{
    float4 Time;
}

cbuffer ShaderParameter : register(b1)
{
    float4x4 World;
    float4x4 View;
    float4x4 Proj;

    float4x4 InvView;
}

struct ParticleData
{
    float3 position;
    float4 color;
};

StructuredBuffer<ParticleData> ParticleDataBuffer : register(t0);

VSOutput main(VSInput In)
{
    VSOutput result = (VSOutput)0;

    float4 worldPos = float4(
        ParticleDataBuffer.Load(In.VertexID).position,
        1.0f
        );
    
    result.Position = worldPos;
    result.Color = In.Color;
    return result;
}