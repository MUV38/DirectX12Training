#pragma once

#include <Application/Application.h>
#include <DirectXMath.h>
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
		CbModel,
		CbNum
	};

public:
    virtual void OnInitialize() override;
    virtual void OnFinalize() override;
    virtual void OnRender(ComPtr<ID3D12GraphicsCommandList>& command) override;

private:
    /// テクスチャの作成
    ComPtr<ID3D12Resource1> CreateTexture(const std::wstring& fileName);

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
    ConstantBuffer m_constantBuffers[CbNum];

    DescriptorHandle m_sampler;
	DescriptorHandle m_srv;
};
