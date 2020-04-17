#pragma once

#include <Application/Application.h>
#include <Model/ModelLoader.h>
#include <Texture/Texture.h>
#include <ConstantBuffer/ConstantBuffer.h>
#include <Shader/ShaderObject.h>

class MyApp : public Application
{
public:
    MyApp();
    virtual ~MyApp();

public:
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
    ModelLoader m_modelLoader;
	Texture m_texture;

	ConstantBuffer m_constantBuffers[CbNum];

	DescriptorHandle m_sampler;

    ShaderObject m_vs, m_ps;

    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12PipelineState> m_pipeline;
};