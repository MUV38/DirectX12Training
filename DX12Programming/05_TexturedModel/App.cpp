#include "App.h"

#undef min
#undef max

#include <stdexcept>
#include <Texture/TextureLoader.h>
#include <Util/D3D12Util.h>

App::App()
{
}

App::~App()
{
}

void App::Prepare()
{
    // モデル読み込み
    m_modelLoader.Load(m_device.Get(), "../assets/shaderball/shaderBall.fbx");

    // シェーダーをコンパイル.
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
    hr = m_device->CreateRootSignature(
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
        psoDesc.SampleMask = UINT_MAX; // これを忘れると絵が出ない&警告も出ない.
    }
    hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipeline));
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
			m_constantBuffers[index] = D3D12Util::CreateBuffer(m_device.Get(), bufferSize, nullptr);

			m_cbViews[index] = m_descriptorManager.Alloc(DescriptorManager::DescriptorPoolType::CbvSrvUav);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbDesc{};
			cbDesc.BufferLocation = m_constantBuffers[index]->GetGPUVirtualAddress();
			cbDesc.SizeInBytes = bufferSize;
			m_device->CreateConstantBufferView(&cbDesc, m_cbViews[index].GetCPUHandle());
		}
	}

	// sampler.
	{
		m_sampler = m_descriptorManager.Alloc(DescriptorManager::DescriptorPoolType::Sampler);

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
		m_device->CreateSampler(&samplerDesc, m_sampler.GetCPUHandle());
	}

	// テクスチャ読み込み
	{
		hr = TextureLoader::LoadDDS(
			m_device.Get(),
			L"../assets/textures/antique/antique_albedo.dds",
			m_descriptorManager.GetDescriptorPool(DescriptorManager::DescriptorPoolType::CbvSrvUav),
			m_commandAllocators[m_frameIndex].Get(),
			m_commandQueue.Get(),
			&m_texture
		);
		if (FAILED(hr))
		{
			throw std::runtime_error("TextureLoader::LoadDDS failed");
		}
	}
}

void App::Cleanup()
{
    auto index = m_swapChain->GetCurrentBackBufferIndex();
    auto fence = m_frameFences[index];
    auto value = ++m_frameFenceValues[index];
    m_commandQueue->Signal(fence.Get(), value);
    fence->SetEventOnCompletion(value, m_fenceWaitEvent);
    WaitForSingleObject(m_fenceWaitEvent, GpuWaitTimeout);
}

void App::MakeCommand(ComPtr<ID3D12GraphicsCommandList>& command)
{
    using namespace DirectX;

    // Matrix.
    ShaderParameters shaderParams;
    XMStoreFloat4x4(&shaderParams.mtxWorld, XMMatrixScaling(0.01f, 0.01f, 0.01f));
    auto mtxView = XMMatrixLookAtLH(
        XMVectorSet(0.f, 1.5f, -5.f, 0.f),
        XMVectorSet(0.f, 1.5f, 0.f, 0.f),
        XMVectorSet(0.f, 1.f, 0.f, 0.f)
    );
    auto mtxProj = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.f), m_viewport.Width / m_viewport.Height, 0.1f, 100.0f);
    XMStoreFloat4x4(&shaderParams.mtxView, XMMatrixTranspose(mtxView));
    XMStoreFloat4x4(&shaderParams.mtxProj, XMMatrixTranspose(mtxProj));

    // Update constant buffer.
    auto& constantBuffer = m_constantBuffers[m_frameIndex];
    {
        void* p;
        D3D12_RANGE range{ 0, 0 };
        constantBuffer->Map(0, &range, &p);
        memcpy(p, &shaderParams, sizeof(shaderParams));
        constantBuffer->Unmap(0, nullptr);
    }

    // PSO.
    command->SetPipelineState(m_pipeline.Get());
    // RootSignature.
    command->SetGraphicsRootSignature(m_rootSignature.Get());
    // Viewport, Scissor.
    command->RSSetViewports(1, &m_viewport);
    command->RSSetScissorRects(1, &m_scissorRect);

    // DescriptorHeap.
	DescriptorPool* cbvSrvUavDescriptorPool = m_descriptorManager.GetDescriptorPool(DescriptorManager::DescriptorPoolType::CbvSrvUav);
	DescriptorPool* samplerDescriptorPool = m_descriptorManager.GetDescriptorPool(DescriptorManager::DescriptorPoolType::Sampler);
	ID3D12DescriptorHeap* heaps[] = {
		cbvSrvUavDescriptorPool->GetHeap(), samplerDescriptorPool->GetHeap()
    };
    command->SetDescriptorHeaps(_countof(heaps), heaps);

    // DescriptorTable.
	int cbvModelIndex = CbvModel * FrameBufferCount + m_frameIndex;
	command->SetGraphicsRootDescriptorTable(0, m_cbViews[cbvModelIndex].GetGPUHandle());
	command->SetGraphicsRootDescriptorTable(1, m_texture.GetShaderResourceView());
	command->SetGraphicsRootDescriptorTable(2, m_sampler.GetGPUHandle());
    
    // DrawModel
    m_modelLoader.Draw(command.Get());
}