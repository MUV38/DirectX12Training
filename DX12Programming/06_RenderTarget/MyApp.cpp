#include "MyApp.h"

#undef min
#undef max

#include <stdexcept>
#include <Texture/TextureLoader.h>
#include <D3D12/D3D12Util.h>
#include <Util/FullScreenQuad.h>

MyApp::MyApp()
{
}

MyApp::~MyApp()
{
}

void MyApp::OnInitialize()
{
	auto* device = GetDevice().Get();
	auto* commandAllocator = GetCommandAllocator().Get();
	auto* commandQueue = GetCommandQueue().Get();
	const auto& viewport = GetViewport();
	auto& descriptorManager = GetDescriptorManager();

	HRESULT hr;
	ComPtr<ID3DBlob> errBlob;

    // ���f���ǂݍ���
    m_modelLoader.Load(device, ASSET_MODEL_ROOT"sponza/sponza.fbx");

    // �V�F�[�_�[���R���p�C��.
	m_vs.compile(L"VertexShader.hlsl", L"vs_6_0");
	m_ps.compile(L"PixelShader.hlsl", L"ps_6_0");

    // DescripterRange.
    CD3DX12_DESCRIPTOR_RANGE cbv, srv, sampler;
    cbv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	srv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	sampler.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);

    // RootParameter.
    CD3DX12_ROOT_PARAMETER rootParams[3];
	rootParams[0].InitAsDescriptorTable(1, &cbv, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParams[1].InitAsDescriptorTable(1, &srv, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParams[2].InitAsDescriptorTable(1, &sampler, D3D12_SHADER_VISIBILITY_PIXEL);

    // RootSignature.
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc{};
    rootSigDesc.Init(
        _countof(rootParams), rootParams,
        0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    );
    ComPtr<ID3DBlob> signature;
    hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &errBlob);
    if (FAILED(hr))
    {
        throw std::runtime_error("D3D12SerializeRootSignature faild.");
    }
    hr = device->CreateRootSignature(
        0,
        signature->GetBufferPointer(), signature->GetBufferSize(),
        IID_PPV_ARGS(&m_rootSignature)
    );
    if (FAILED(hr))
    {
        throw std::runtime_error("CrateRootSignature failed.");
    }

    // InputLayout.
    D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, pos), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, uv), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA },
    };

    // PipelineStateObject.
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    {
        // Shader.
        psoDesc.VS = m_vs.getShaderByteCode();
        psoDesc.PS = m_ps.getShaderByteCode();
        // BlendState.
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        // RasterizerState.
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        // RenderTarget.
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        // DepthStencil.
        psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        // InputLayout.
        psoDesc.InputLayout = { inputElementDesc, _countof(inputElementDesc) };

        // RootSignature.
        psoDesc.pRootSignature = m_rootSignature.Get();
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        // MultiSample.
        psoDesc.SampleDesc = { 1, 0 };
        psoDesc.SampleMask = UINT_MAX; // �����Y���ƊG���o�Ȃ�&�x�����o�Ȃ�.
    }
    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipeline));
    if (FAILED(hr))
    {
        throw std::runtime_error("CreateGraphicsPipelineState failed");
    }

    // ConstantBuffer/View.
	{
		m_constantBuffers[CbModel].create(device, &descriptorManager, sizeof(ShaderParameters));
	}

	// sampler.
	{
		m_sampler = descriptorManager.Alloc(DescriptorManager::DescriptorPoolType::Sampler);

		// sampler desc.
		D3D12_SAMPLER_DESC samplerDesc{};
		{
			samplerDesc.Filter = D3D12_ENCODE_BASIC_FILTER(
				D3D12_FILTER_TYPE_LINEAR, // min
				D3D12_FILTER_TYPE_LINEAR, // mag
				D3D12_FILTER_TYPE_LINEAR, // mip
				D3D12_FILTER_REDUCTION_TYPE_STANDARD
			);
			samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			samplerDesc.MaxLOD = FLT_MAX;
			samplerDesc.MinLOD = -FLT_MAX;
			samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		}
		device->CreateSampler(&samplerDesc, m_sampler.GetCPUHandle());
	}

	// Render Target
	{
		{
			RenderTarget::SetupParam setupParam;
			{
				setupParam.width = static_cast<UINT>(viewport.Width);
				setupParam.height = static_cast<UINT>(viewport.Height);
				setupParam.format = DXGI_FORMAT_R8G8B8A8_UNORM;

				setupParam.rtvDescriptorPool = descriptorManager.GetDescriptorPool(DescriptorManager::DescriptorPoolType::Rtv);
				setupParam.cbvSrvUavDescriptorPool = descriptorManager.GetDescriptorPool(DescriptorManager::DescriptorPoolType::CbvSrvUav);

				setupParam.allowUav = false;
			}
			m_rt[RtTmp].Setup(device, setupParam);
		}
	}

	// �e�N�X�`���ǂݍ���
	{
		hr = TextureLoader::LoadDDS(
			device,
			ASSET_TEXTURE_ROOT"uvChecker.dds",
			descriptorManager.GetDescriptorPool(DescriptorManager::DescriptorPoolType::CbvSrvUav),
			commandAllocator,
			commandQueue,
			&m_texture[TexAlbedo]
		);
		if (FAILED(hr))
		{
			throw std::runtime_error("TextureLoader::LoadDDS failed");
		}
	}

	// �t���X�N���[���`��
	{
		FullScreenQuad::SetupParam setupParam;
		{
			setupParam.SetupRootSignatureOneTexture(device);
		}
		m_fullScreenQuad.Setup(device, setupParam);
	}
}

void MyApp::OnFinalize()
{
	WaitForGPU();
}

void MyApp::OnRender(ComPtr<ID3D12GraphicsCommandList>& command)
{
    using namespace DirectX;

	const auto& viewport = GetViewport();
	const auto& scissorRect = GetScissorRect();
	auto frameIndex = GetFrameIndex();
	auto& descriptorManager = GetDescriptorManager();
	auto* commandList = GetCommandList().Get();

	auto& cbModel = m_constantBuffers[CbModel];

    // Matrix.
    ShaderParameters shaderParams;
    XMStoreFloat4x4(&shaderParams.mtxWorld, XMMatrixScaling(0.01f, 0.01f, 0.01f));
    auto mtxView = XMMatrixLookAtLH(
        XMVectorSet(-5.f, 1.5f, 0.f, 0.f),
        XMVectorSet(0.f, 1.5f, 0.f, 0.f),
        XMVectorSet(0.f, 1.f, 0.f, 0.f)
    );
    auto mtxProj = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.f), viewport.Width / viewport.Height, 0.1f, 100.0f);
    XMStoreFloat4x4(&shaderParams.mtxView, XMMatrixTranspose(mtxView));
    XMStoreFloat4x4(&shaderParams.mtxProj, XMMatrixTranspose(mtxProj));

    // Update constant buffer.
	{
		void* p = nullptr;
		cbModel.map(frameIndex, &p);
		memcpy(p, &shaderParams, sizeof(shaderParams));
		cbModel.unmap(frameIndex);
	}

	// RenderTarget(tmp)
	{
		INT rtvHandleOffset = RtTmp;
		D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_rt[RtTmp].GetRtvDescriptorHandle().GetCPUHandle();
		D3D12_CPU_DESCRIPTOR_HANDLE dsv = GetBackBufferDsvDescriptorHandle().GetCPUHandle();
		// �����_�[�^�[�Q�b�g�̐ݒ�
		command->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
		// �N���A
		const float clearColor[] = { 0.3f, 0.3f, 0.3f, 1.0f };
		command->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
		command->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}
    // PSO.
    command->SetPipelineState(m_pipeline.Get());
    // RootSignature.
    command->SetGraphicsRootSignature(m_rootSignature.Get());
    // Viewport, Scissor.
    command->RSSetViewports(1, &viewport);
    command->RSSetScissorRects(1, &scissorRect);

    // DescriptorHeap.
	DescriptorPool* srvCbvUavHeap = descriptorManager.GetDescriptorPool(DescriptorManager::DescriptorPoolType::CbvSrvUav);
	DescriptorPool* samplerHeap = descriptorManager.GetDescriptorPool(DescriptorManager::DescriptorPoolType::Sampler);
	ID3D12DescriptorHeap* heaps[] = {
		srvCbvUavHeap->GetHeap(), samplerHeap->GetHeap()
    };
    command->SetDescriptorHeaps(_countof(heaps), heaps);

    // DescriptorTable.
	{
		command->SetGraphicsRootDescriptorTable(0, cbModel.getView(frameIndex));
		command->SetGraphicsRootDescriptorTable(1, m_texture[TexAlbedo].GetShaderResourceView());
		command->SetGraphicsRootDescriptorTable(2, m_sampler.GetGPUHandle());
	}
    
    // DrawModel
    m_modelLoader.Draw(command.Get());

	// RenderTarget(back buffer)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE rtv = GetBackBufferRtvDescriptorHandle().GetCPUHandle();
		D3D12_CPU_DESCRIPTOR_HANDLE dsv = GetBackBufferDsvDescriptorHandle().GetCPUHandle();
		// �����_�[�^�[�Q�b�g�̐ݒ�
		commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
		// �N���A
		const float clearColor[] = { 0.3f, 0.3f, 0.3f, 1.0f };
		commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
		commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

	// �����_�[�^�[�Q�b�g�`��\����s�N�Z���V�F�[�_�[���\�[�X��
	m_rt[RtTmp].SetResourceBarrier(command.Get(), RenderTarget::ResourceBarrierType::PixelShaderResource);
	
	// �t���X�N���[���`��
	m_fullScreenQuad.SetPipelineState(command.Get());
	m_fullScreenQuad.SetGraphicsRootSignature(command.Get());
	command->SetGraphicsRootDescriptorTable(0, m_rt[RtTmp].GetShaderResourceView());
	command->SetGraphicsRootDescriptorTable(1, m_sampler.GetGPUHandle());
	m_fullScreenQuad.Draw(command.Get());

	// �s�N�Z���V�F�[�_�[���\�[�X���烌���_�[�^�[�Q�b�g�`��\��
	m_rt[RtTmp].SetResourceBarrier(command.Get(), RenderTarget::ResourceBarrierType::RenderTarget);
}