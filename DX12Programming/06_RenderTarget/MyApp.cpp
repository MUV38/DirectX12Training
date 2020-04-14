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

    // ���f���ǂݍ���
    m_modelLoader.Load(device, "../assets/shaderball/shaderBall.fbx");

    // �V�F�[�_�[���R���p�C��.
    HRESULT hr;
    ComPtr<ID3DBlob> errBlob;
	hr = D3D12Util::CompileShaderFromFile(L"VertexShader.hlsl", L"vs_6_0", m_vs, errBlob);
    if (FAILED(hr))
    {
        OutputDebugStringA(static_cast<const char*>(errBlob->GetBufferPointer()));
    }
    hr = D3D12Util::CompileShaderFromFile(L"PixelShader.hlsl", L"ps_6_0", m_ps, errBlob);
    if (FAILED(hr))
    {
        OutputDebugStringA(static_cast<const char*>(errBlob->GetBufferPointer()));
    }

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
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vs.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_ps.Get());
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
    m_constantBuffers.resize(CbvNum * FrameBufferCount);
    m_cbViews.resize(CbvNum * FrameBufferCount);
	for (UINT i = 0; i < CbvNum; ++i)
	{
		for (UINT j = 0; j < FrameBufferCount; ++j)
		{
			int index = i * FrameBufferCount + j;

			UINT bufferSize = sizeof(ShaderParameters) + 255 & ~255;
			m_constantBuffers[index] = D3D12Util::CreateBuffer(device, bufferSize, nullptr);

			m_cbViews[index] = descriptorManager.Alloc(DescriptorManager::DescriptorPoolType::CbvSrvUav);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbDesc{};
			cbDesc.BufferLocation = m_constantBuffers[index]->GetGPUVirtualAddress();
			cbDesc.SizeInBytes = bufferSize;
			device->CreateConstantBufferView(&cbDesc, m_cbViews[index].GetCPUHandle());
		}
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
		m_renderTargets.resize(RtNum);
		m_rtSrvs.resize(RtNum);
		m_rtvs.resize(RtNum);
		
		// RtvGrayScale
		{
			// resource
			ComPtr<ID3D12Resource1> resource;
			DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
			CD3DX12_RESOURCE_DESC resDesc(
				D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				0,
				static_cast<UINT>(viewport.Width),
				static_cast<UINT>(viewport.Height),
				1,
				1,
				format,
				1,
				0,
				D3D12_TEXTURE_LAYOUT_UNKNOWN,
				D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
			);
			D3D12_CLEAR_VALUE clearValue;
			{
				FLOAT color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
				memcpy(&clearValue.Color, color, sizeof(FLOAT) * 4);
				clearValue.Format = format;
			}

			hr = device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&resDesc,
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				&clearValue,
				IID_PPV_ARGS(resource.GetAddressOf())
			);
			if (FAILED(hr))
			{
				throw std::runtime_error("RenderTarget CreateCommittedResource failed");
			}
			m_renderTargets[RtTmp] = resource;

			// RTV
			m_rtvs[RtTmp] = descriptorManager.Alloc(DescriptorManager::DescriptorPoolType::Rtv);
			m_rtSrvs[RtTmp] = descriptorManager.Alloc(DescriptorManager::DescriptorPoolType::CbvSrvUav);

			D3D12_RENDER_TARGET_VIEW_DESC rtViewDesc = {};
			{
				rtViewDesc.Format = format;
				rtViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			}
			device->CreateRenderTargetView(resource.Get(), &rtViewDesc, m_rtvs[RtTmp].GetCPUHandle());
			// SRV
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			{
				srvDesc.Texture2D.MipLevels = resDesc.MipLevels;
				srvDesc.Format = format;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			}
			device->CreateShaderResourceView(resource.Get(), &srvDesc, m_rtSrvs[RtTmp].GetCPUHandle());
		}
	}

	// �e�N�X�`���ǂݍ���
	{
		hr = TextureLoader::LoadDDS(
			device,
			L"../assets/textures/antique/antique_albedo.dds",
			descriptorManager.GetDescriptorPool(DescriptorManager::DescriptorPoolType::CbvSrvUav),
			commandAllocator,
			commandQueue,
			&m_texture
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

    // Matrix.
    ShaderParameters shaderParams;
    XMStoreFloat4x4(&shaderParams.mtxWorld, XMMatrixScaling(0.01f, 0.01f, 0.01f));
    auto mtxView = XMMatrixLookAtLH(
        XMVectorSet(0.f, 1.5f, -5.f, 0.f),
        XMVectorSet(0.f, 1.5f, 0.f, 0.f),
        XMVectorSet(0.f, 1.f, 0.f, 0.f)
    );
    auto mtxProj = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.f), viewport.Width / viewport.Height, 0.1f, 100.0f);
    XMStoreFloat4x4(&shaderParams.mtxView, XMMatrixTranspose(mtxView));
    XMStoreFloat4x4(&shaderParams.mtxProj, XMMatrixTranspose(mtxProj));

    // Update constant buffer.
    auto& constantBuffer = m_constantBuffers[frameIndex];
    {
        void* p;
        D3D12_RANGE range{ 0, 0 };
        constantBuffer->Map(0, &range, &p);
        memcpy(p, &shaderParams, sizeof(shaderParams));
        constantBuffer->Unmap(0, nullptr);
    }

	// RenderTarget(tmp)
	{
		INT rtvHandleOffset = RtTmp;
		D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_rtvs[rtvHandleOffset].GetCPUHandle();
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
	int cbvModelIndex = CbvModel * FrameBufferCount + frameIndex;
	command->SetGraphicsRootDescriptorTable(0, m_cbViews[cbvModelIndex].GetGPUHandle());
	command->SetGraphicsRootDescriptorTable(1, m_texture.GetShaderResourceView());
	command->SetGraphicsRootDescriptorTable(2, m_sampler.GetGPUHandle());
    
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
	auto barrierToPixelShaderResource = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[RtTmp].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	command->ResourceBarrier(1, &barrierToPixelShaderResource);
	
	// �t���X�N���[���`��
	m_fullScreenQuad.SetPipelineState(command.Get());
	m_fullScreenQuad.SetGraphicsRootSignature(command.Get());
	command->SetGraphicsRootDescriptorTable(0, m_rtSrvs[RtTmp].GetGPUHandle());
	command->SetGraphicsRootDescriptorTable(1, m_sampler.GetGPUHandle());
	m_fullScreenQuad.Draw(command.Get());

	// �s�N�Z���V�F�[�_�[���\�[�X���烌���_�[�^�[�Q�b�g�`��\��
	auto barrierToRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[RtTmp].Get(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	command->ResourceBarrier(1, &barrierToRenderTarget);
}