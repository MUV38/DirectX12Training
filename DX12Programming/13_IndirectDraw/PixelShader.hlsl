struct VSOutput
{
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
    float4 Color    : COLOR;
};

float4 main(VSOutput In) : SV_TARGET
{
    const float3 lightDir = normalize(float3(0.0f, -0.5f, 1.0f));
    
    float intensity = max(dot(-lightDir, In.Normal), 0.3f);
    return float4(In.Color.rgb * intensity, 1.0f);
}