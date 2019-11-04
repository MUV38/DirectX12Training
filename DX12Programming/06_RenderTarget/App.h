#pragma once

#include <D3D12/D3D12AppBase.h>
#include <Model/ModelLoader.h>
#include <Texture/Texture.h>

class App : public D3D12AppBase
{
public:
	App();
	virtual ~App();

public:
    struct ShaderParameters
    {
        DirectX::XMFLOAT4X4 mtxWorld;
        DirectX::XMFLOAT4X4 mtxView;
        DirectX::XMFLOAT4X4 mtxProj;
    };

	enum
	{
		TexSrvAlbedo,
		TexSrvNum
	};

	enum
	{
		CbvModel,
		CbvNum
	};

	enum
	{
		RtGrayScale,
		RtNum
	};

	enum
	{
		// ---CBV_SRV_UAV---
		TexSrvDescriptorBase = 0,
		TexSrvDescriptorEnd = TexSrvDescriptorBase + (TexSrvNum - 1),
		
		CbvDescriptorBase,
		CbvDescriptorEnd = CbvDescriptorBase + (CbvNum * FrameBufferCount - 1),

		RtSrvDescriptorBase,
		RtSrvDescriptorEnd = RtSrvDescriptorBase + (RtNum - 1),

		CBV_SRV_UAV_Num,
		// ---End of CBV_SRV_UAV---

		// ---Sampler---
        SamplerDescriptorBase = 0,
		// ---End of Sampler---

		// ---RTV---
		RtvDescriptorBase = 0,
		RtvDescriptorEnd = RtvDescriptorBase + (RtNum - 1),
		// ---End of RTV---
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

	std::vector<ComPtr<ID3D12Resource>> m_constantBuffers;
	std::vector<ComPtr<ID3D12Resource>> m_renderTargets;

    ComPtr<ID3D12DescriptorHeap> m_heapSrvCbv;
	ComPtr<ID3D12DescriptorHeap> m_heapSampler;
	ComPtr<ID3D12DescriptorHeap> m_heapRtv;
	UINT m_samplerDescriptorSize;

    std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_cbViews;
	D3D12_GPU_DESCRIPTOR_HANDLE m_sampler;
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_rtvViews;
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_rtSRViews;

    ComPtr<ID3DBlob> m_vs, m_ps;

    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12PipelineState> m_pipeline;
};