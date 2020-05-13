#pragma once

#include <Application/Application.h>
#include <Model/ModelLoader.h>
#include <Texture/Texture.h>
#include <Util/FullScreenQuad.h>
#include <RenderTarget/RenderTarget.h>
#include <ConstantBuffer/ConstantBuffer.h>
#include <Shader/ShaderObject.h>

#define NUM_PARTICLE (1000)

class MyApp : public Application
{
public:
	MyApp();
	virtual ~MyApp();

public:
	struct Vertex
	{
		DirectX::XMFLOAT4 color;
	};

	struct ParticleData
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};

	struct ParticleGlobalParam
	{
		DirectX::XMFLOAT4 velocityParam; // x:scale
	};

	struct SceneParameters
	{
		DirectX::XMFLOAT4 time; // x:elapsedTime y:deltaTime
	};

    struct ShaderParameters
    {
        DirectX::XMFLOAT4X4 mtxWorld;
        DirectX::XMFLOAT4X4 mtxView;
        DirectX::XMFLOAT4X4 mtxProj;

		DirectX::XMFLOAT4X4 mtxInvView;
    };

	enum
	{
		TexAlbedo,
		TexNum
	};

	enum
	{
		CbScene,
		CbModel,
		CbParticle,
		CbNum
	};

	enum
	{
		RtTmp,
		RtNum
	};

public:
    virtual void OnInitialize() override;
    virtual void OnFinalize() override;
	virtual void OnUpdate(float deltaTime) override;
    virtual void OnRender(ComPtr<ID3D12GraphicsCommandList>& command) override;

private:
	// GUI更新
	void updateGUI(float deltaTime);

	// シェーダーパラメータ更新
	void updateShaderParam(float deltaTime);

	// コンスタントバッファ更新
	void updateConstantBuffer(float deltaTime);

private:
	Texture m_texture[TexNum];

	ConstantBuffer m_constantBuffers[CbNum];

	DescriptorHandle m_sampler;

	ShaderObject m_vs, m_gs, m_ps, m_cs;

    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12PipelineState> m_pipeline;

	ComPtr<ID3D12RootSignature> m_computeRootSignature;
	ComPtr<ID3D12PipelineState> m_computePipeline;

	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	ComPtr<ID3D12Resource> m_particleBuffer;
	DescriptorHandle m_particleSrv;
	DescriptorHandle m_particleUav;

	SceneParameters m_sceneParam;
	ParticleGlobalParam m_particleGlobalParam;
};