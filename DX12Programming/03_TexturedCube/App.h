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
        DirectX::XMFLOAT2 uv;
    };

    struct ShaderParameters
    {
        DirectX::XMFLOAT4X4 mtxWorld;
        DirectX::XMFLOAT4X4 mtxView;
        DirectX::XMFLOAT4X4 mtxProj;
    };

	enum
	{
		CbvModel,
		CbvNum
	};

public:
    virtual void OnInitialize() override;
    virtual void OnFinalize() override;
    virtual void OnRender(ComPtr<ID3D12GraphicsCommandList>& command) override;

private:
    /// テクスチャの作成
    ComPtr<ID3D12Resource1> CreateTexture(const std::string& fileName);

private:    
    ComPtr<ID3D12Resource> m_vertexBuffer;
    ComPtr<ID3D12Resource> m_indexBuffer;
    ComPtr<ID3D12Resource> m_texture;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
    UINT m_indexCount;

    ComPtr<ID3DBlob> m_vs, m_ps;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12PipelineState> m_pipeline;
    std::vector < ComPtr<ID3D12Resource>> m_constantBuffers;

    DescriptorHandle m_sampler;
	DescriptorHandle m_srv;
    std::vector<DescriptorHandle> m_cbViews;
};
