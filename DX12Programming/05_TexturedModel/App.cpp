#include "App.h"

#undef min
#undef max

#include <stdexcept>

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
    hr = CompileShaderFromFile(L"VertexShader.hlsl", L"vs_6_0", m_vs, errBlob);
    if (FAILED(hr))
    {
        OutputDebugStringA(static_cast<const char*>(errBlob->GetBufferPointer()));
    }
    hr = CompileShaderFromFile(L"PixelShader.hlsl", L"ps_6_0", m_ps, errBlob);
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

    PrepareDescriptorHeapForModelApp();

    // ConstantBuffer/View.
    m_constantBuffers.resize(FrameBufferCount);
    m_cbViews.resize(FrameBufferCount);
    for (UINT i = 0; i < FrameBufferCount; ++i)
    {
        UINT bufferSize = sizeof(ShaderParameters) + 255 & ~255;
        m_constantBuffers[i] = CreateBuffer(bufferSize, nullptr);

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbDesc{};
        cbDesc.BufferLocation = m_constantBuffers[i]->GetGPUVirtualAddress();
        cbDesc.SizeInBytes = bufferSize;
        CD3DX12_CPU_DESCRIPTOR_HANDLE handleCBV(m_heapSrvCbv->GetCPUDescriptorHandleForHeapStart(), ConstantBufferDescriptorBase + i, m_srvcbvDescriptorSize);
        m_device->CreateConstantBufferView(&cbDesc, handleCBV);

        m_cbViews[i] = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_heapSrvCbv->GetGPUDescriptorHandleForHeapStart(), ConstantBufferDescriptorBase + i, m_srvcbvDescriptorSize);
    }

	// sampler.
	{
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

		// DescriptorHeap.
		auto descriptorSampler = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_heapSampler->GetCPUDescriptorHandleForHeapStart(), SamplerDescriptorBase, m_samplerDescriptorSize);
		m_device->CreateSampler(&samplerDesc, descriptorSampler);
		m_sampler = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_heapSampler->GetGPUDescriptorHandleForHeapStart(), SamplerDescriptorBase, m_samplerDescriptorSize);
	}

	// テクスチャ読み込み
	{
		DirectX::TexMetadata metadata;
		auto image = std::make_unique<DirectX::ScratchImage>();
		HRESULT hr = DirectX::LoadFromDDSFile(
			L"../assets/textures/antique/antique_albedo.dds",
			DirectX::DDS_FLAGS_NONE,
			&metadata, *image
		);
		if (FAILED(hr))
		{
			throw std::runtime_error("DirectXLoadFromDDSFile faild.");
		}

		hr = DirectX::CreateTexture(m_device.Get(), metadata, &m_texture);
		if (FAILED(hr))
		{
			throw std::runtime_error("CreateTexture faild.");
		}

		std::vector<D3D12_SUBRESOURCE_DATA> subresources;
		hr = DirectX::PrepareUpload(m_device.Get(), image->GetImages(), image->GetImageCount(), metadata, subresources);
		if (FAILED(hr))
		{
			throw std::runtime_error("PrepareUpload faild.");
		}

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, static_cast<unsigned int>(subresources.size()));

		ComPtr<ID3D12Resource> textureUploadHeap;
		hr = m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(textureUploadHeap.GetAddressOf())
		);
		if (FAILED(hr))
		{
			throw std::runtime_error("CreateCommittedResource faild.");
		}

		// コマンド準備
		ComPtr<ID3D12GraphicsCommandList> command;
		m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_frameIndex].Get(), nullptr, IID_PPV_ARGS(&command));
		ComPtr<ID3D12Fence1> fence;
		m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

		UpdateSubresources(command.Get(), m_texture.Get(), textureUploadHeap.Get(), 0, 0, static_cast<unsigned int>(subresources.size()), subresources.data());

		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_texture.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		);
		command->ResourceBarrier(1, &barrier);

		command->Close();

		// コマンド実行
		ID3D12CommandList* cmds[] = { command.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(cmds), cmds);

		// 完了したらシグナルを立てる
		const UINT64 expected = 1;
		m_commandQueue->Signal(fence.Get(), expected);

		// テクスチャの処理が完了するまで待つ
		while (expected != fence->GetCompletedValue())
		{
			Sleep(1);
		}

		// texture srv.
		{
			auto srvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_heapSrvCbv->GetCPUDescriptorHandleForHeapStart(), TextureSrvDescriptorBase, m_srvcbvDescriptorSize);
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			{
				srvDesc.Texture2D.MipLevels = metadata.mipLevels;
				srvDesc.Format = metadata.format;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			}
			m_device->CreateShaderResourceView(m_texture.Get(), &srvDesc, srvHandle);
			m_textureSrv = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_heapSrvCbv->GetGPUDescriptorHandleForHeapStart(), TextureSrvDescriptorBase, m_srvcbvDescriptorSize);
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
    ID3D12DescriptorHeap* heaps[] = {
        m_heapSrvCbv.Get(), m_heapSampler.Get()
    };
    command->SetDescriptorHeaps(_countof(heaps), heaps);

    // DescriptorTable.
	command->SetGraphicsRootDescriptorTable(0, m_cbViews[m_frameIndex]);
	command->SetGraphicsRootDescriptorTable(1, m_textureSrv);
	command->SetGraphicsRootDescriptorTable(2, m_sampler);
    
    // DrawModel
    m_modelLoader.Draw(command.Get());
}

App::ComPtr<ID3D12Resource1> App::CreateBuffer(UINT bufferSize, const void* initialData)
{
    HRESULT hr;
    ComPtr<ID3D12Resource1> buffer;
    hr = m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&buffer)
    );

    // 初期データがあればコピー.
    if (SUCCEEDED(hr) && initialData)
    {
        void* mapped;
        CD3DX12_RANGE range(0, 0);
        hr = buffer->Map(0, &range, &mapped);
        if (SUCCEEDED(hr))
        {
            memcpy(mapped, initialData, bufferSize);
            buffer->Unmap(0, nullptr);
        }
    }

    return buffer;
}

void App::PrepareDescriptorHeapForModelApp()
{
    // ディスクリプターヒープ
    UINT count = FrameBufferCount + 1;
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        count,
        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
        0
    };
    m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_heapSrvCbv));
    m_srvcbvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// ダイナミックサンプラーのディスクリプターヒープ
	D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc{
		D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
		1,
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		0
	};
	m_device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_heapSampler));
	m_samplerDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}
