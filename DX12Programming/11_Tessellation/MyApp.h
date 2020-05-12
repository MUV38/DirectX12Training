#pragma once

#include <Application/Application.h>
#include <DirectXMath.h>
#include <Shader/ShaderObject.h>
#include <ConstantBuffer/ConstantBuffer.h>

class MyApp : public Application
{
public:
    MyApp();
    virtual ~MyApp();

public:
    struct Vertex
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT4 color;
    };

    struct TessellationParameters
    {
        DirectX::XMFLOAT4 TessFactor; // x:edge0 y:edge1 z:edge2 w:inside
        DirectX::XMFLOAT4 TessFactor2; // x:subdivision
    };

public:
    virtual void OnInitialize() override;
    virtual void OnFinalize() override;
    virtual void OnUpdate(float deltaTime) override;
    virtual void OnRender(ComPtr<ID3D12GraphicsCommandList>& command) override;

private:
    enum
    {
        CbTessellation,
        CbNum
    };

private:
    /// バッファの作成
    ComPtr<ID3D12Resource1> CreateBuffer(UINT bufferSize, const void* initialData);

    // GUI更新
    void updateGUI(float deltaTime);

    // コンスタントバッファ更新
    void updateConstantBuffer(float deltaTime);

private:
    ComPtr<ID3D12Resource1> m_vertexBuffer;
    ComPtr<ID3D12Resource1> m_indexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
    UINT m_indexCount;

    ShaderObject m_vs, m_hs, m_ds, m_ps;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12PipelineState> m_pipeline;

    ConstantBuffer m_constantBuffers[CbNum];

    TessellationParameters m_tessParam;
};
