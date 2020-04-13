#pragma once

#include <D3D12/D3D12AppBase.h>
#include <Model/ModelLoader.h>
#include <Texture/Texture.h>
#include <Util/FullScreenQuad.h>
#include <RenderTarget/RenderTarget.h>

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
		RtTmp,
		RtNum
	};

public:
    virtual void OnInitialize() override;
    virtual void OnFinalize() override;
    virtual void OnRender(ComPtr<ID3D12GraphicsCommandList>& command) override;

private:
    ModelLoader m_modelLoader;
	Texture m_texture;

	std::vector<ComPtr<ID3D12Resource>> m_constantBuffers;

    std::vector<DescriptorHandle> m_cbViews;
	DescriptorHandle m_sampler;

    ComPtr<ID3DBlob> m_vs, m_ps, m_cs;

    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12PipelineState> m_pipeline;

	FullScreenQuad m_fullScreenQuad;

	ComPtr<ID3D12RootSignature> m_computeRootSignature;
	ComPtr<ID3D12PipelineState> m_computePipeline;

	RenderTarget m_rtTmp;
};