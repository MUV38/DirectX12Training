#pragma once

#include <D3D12/D3D12AppBase.h>
#include <Model/ModelLoader.h>
#include <Texture/Texture.h>

class App : public D3D12AppBase
{
public:
	App();
    ~App();

public:
    struct ShaderParameters
    {
        DirectX::XMFLOAT4X4 mtxWorld;
        DirectX::XMFLOAT4X4 mtxView;
        DirectX::XMFLOAT4X4 mtxProj;
    };

    enum
    {
		TextureSrvDescriptorBase = 0,
		ConstantBufferDescriptorBase = 1,

        SamplerDescriptorBase = 0,
    };

public:
    virtual void Prepare() override;
    virtual void Cleanup() override;
    virtual void MakeCommand(ComPtr<ID3D12GraphicsCommandList>& command) override;

private:
    /// バッファの作成
    ComPtr<ID3D12Resource1> CreateBuffer(UINT bufferSize, const void* initialData);

    /// ディスクリプタヒープのセットアップ
    void PrepareDescriptorHeapForModelApp();

private:
    ModelLoader m_modelLoader;
	Texture m_texture;

	std::vector<ComPtr<ID3D12Resource1>> m_constantBuffers;

    ComPtr<ID3D12DescriptorHeap> m_heapSrvCbv;
	ComPtr<ID3D12DescriptorHeap> m_heapSampler;
	UINT m_samplerDescriptorSize;

    std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_cbViews;
	D3D12_GPU_DESCRIPTOR_HANDLE m_sampler;

    ComPtr<ID3DBlob> m_vs, m_ps;

    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12PipelineState> m_pipeline;
};