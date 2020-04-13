#pragma once

#include <D3D12/D3D12AppBase.h>
#include <DirectXMath.h>

class App : public D3D12AppBase
{
public:
    App();
    virtual ~App();

public:
    struct Vertex
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT4 color;
    };

public:
    virtual void OnInitialize() override;
    virtual void OnFinalize() override;
    virtual void OnRender(ComPtr<ID3D12GraphicsCommandList>& command) override;

private:
    /// バッファの作成
    ComPtr<ID3D12Resource1> CreateBuffer(UINT bufferSize, const void* initialData);

private:
    ComPtr<ID3D12Resource1> m_vertexBuffer;
    ComPtr<ID3D12Resource1> m_indexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
    UINT m_indexCount;

    ComPtr<ID3DBlob> m_vs, m_ps;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12PipelineState> m_pipeline;
};
