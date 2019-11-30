#include "CubeApp.h"
#include <stdexcept>
#define STB_IMAGE_IMPLEMENTATION
#include <ThirdParty/stb/stb_image.h>
#include <Util/D3D12Util.h>

CubeApp::CubeApp()
{
}

CubeApp::~CubeApp()
{
}

void CubeApp::Prepare()
{
	const float k = 1.0f;
	const DirectX::XMFLOAT4 red(1.f, 0.f, 0.f, 1.f);
	const DirectX::XMFLOAT4 green(0.f, 1.f, 0.f, 1.f);
	const DirectX::XMFLOAT4 blue(0.f, 0.f, 1.f, 1.f);
	const DirectX::XMFLOAT4 white(1.f, 1.f, 1.f, 1.f);
	const DirectX::XMFLOAT4 black(0.f, 0.f, 0.f, 1.f);
	const DirectX::XMFLOAT4 yellow(1.f, 1.f, 0.f, 1.f);
	const DirectX::XMFLOAT4 magenta(1.f, 0.f, 1.f, 1.f);
	const DirectX::XMFLOAT4 cyan(0.f, 1.f, 1.f, 1.f);

	Vertex cubeVertices[] = {
		{ {-k,-k,-k}, red, { 0.0f, 1.0f} },
		{ {-k, k,-k}, yellow, { 0.0f, 0.0f} },
		{ { k, k,-k}, white, { 1.0f, 0.0f} },
		{ { k,-k,-k}, magenta, { 1.0f, 1.0f} },
		// 右
		{ { k,-k,-k}, magenta, { 0.0f, 1.0f} },
		{ { k, k,-k}, white, { 0.0f, 0.0f} },
		{ { k, k, k}, cyan, { 1.0f, 0.0f} },
		{ { k,-k, k}, blue, { 1.0f, 1.0f} },
		// 左
		{ {-k,-k, k}, black, { 0.0f, 1.0f} },
		{ {-k, k, k}, green, { 0.0f, 0.0f} },
		{ {-k, k,-k}, yellow, { 1.0f, 0.0f} },
		{ {-k,-k,-k}, red, { 1.0f, 1.0f} },
		// 裏
		{ { k,-k, k}, blue, { 0.0f, 1.0f} },
		{ { k, k, k}, cyan, { 0.0f, 0.0f} },
		{ {-k, k, k}, green, { 1.0f, 0.0f} },
		{ {-k,-k, k}, black, { 1.0f, 1.0f} },
		// 上
		{ {-k, k,-k}, yellow, { 0.0f, 1.0f} },
		{ {-k, k, k}, green, { 0.0f, 0.0f} },
		{ { k, k, k}, cyan, { 1.0f, 0.0f} },
		{ { k, k,-k}, white, { 1.0f, 1.0f} },
		// 底
		{ {-k,-k, k}, red, { 0.0f, 1.0f} },
		{ {-k,-k,-k}, red, { 0.0f, 0.0f} },
		{ { k,-k,-k}, magenta, { 1.0f, 0.0f} },
		{ { k,-k, k}, blue, { 1.0f, 1.0f} },
	};
	uint32_t indices[] = {
	  0, 1, 2, 2, 3,0,
	  4, 5, 6, 6, 7,4,
	  8, 9, 10, 10, 11, 8,
	  12,13,14, 14,15,12,
	  16,17,18, 18,19,16,
	  20,21,22, 22,23,20,
	};

	// 頂点バッファとインデックスバッファの生成.
	m_vertexBuffer = D3D12Util::CreateBuffer(m_device.Get(), sizeof(cubeVertices), cubeVertices);
	m_indexBuffer = D3D12Util::CreateBuffer(m_device.Get(), sizeof(indices), indices);
	m_indexCount = _countof(indices);

	// 各バッファのビューを生成.
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = sizeof(cubeVertices);
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);
	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.SizeInBytes = sizeof(indices);
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;

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
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA },
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
	m_constantBuffers.resize(CbvNum* FrameBufferCount);
	m_cbViews.resize(CbvNum* FrameBufferCount);
	for (int i = 0; i < CbvNum; ++i)
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

	// texture.
	m_texture = CreateTexture("assets/texture/texture.tga");

	// sampler.
	{
		m_sampler = m_descriptorManager.Alloc(DescriptorManager::DescriptorPoolType::Sampler);

		D3D12_SAMPLER_DESC samplerDesc{};
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

		// DescriptorHeap.
		m_device->CreateSampler(&samplerDesc, m_sampler.GetCPUHandle());
	}

	// CreateShaderResourceView from texture.
	{
		m_srv = m_descriptorManager.Alloc(DescriptorManager::DescriptorPoolType::CbvSrvUav);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		m_device->CreateShaderResourceView(m_texture.Get(), &srvDesc, m_srv.GetCPUHandle());
	}
}

void CubeApp::Cleanup()
{
    auto index = m_swapChain->GetCurrentBackBufferIndex();
    auto fence = m_frameFences[index];
    auto value = ++m_frameFenceValues[index];
    m_commandQueue->Signal(fence.Get(), value);
    fence->SetEventOnCompletion(value, m_fenceWaitEvent);
    WaitForSingleObject(m_fenceWaitEvent, GpuWaitTimeout);
}

void CubeApp::MakeCommand(ComPtr<ID3D12GraphicsCommandList>& command)
{
    using namespace DirectX;

    // Matrix.
    ShaderParameters shaderParams;
    XMStoreFloat4x4(&shaderParams.mtxWorld, XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(45.f)));
    auto mtxView = XMMatrixLookAtLH(
        XMVectorSet(0.f, 3.f, -5.f, 0.f),
        XMVectorSet(0.f, 0.f,  0.f, 0.f),
        XMVectorSet(0.f, 1.f,  0.f, 0.f)
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

    // PrimitiveType, Vertex, Index.
    command->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    command->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    command->IASetIndexBuffer(&m_indexBufferView);

	int cbvModelIndex = CbvModel * FrameBufferCount + m_frameIndex;
    command->SetGraphicsRootDescriptorTable(0, m_cbViews[cbvModelIndex].GetGPUHandle());
    command->SetGraphicsRootDescriptorTable(1, m_srv.GetGPUHandle());
    command->SetGraphicsRootDescriptorTable(2, m_sampler.GetGPUHandle());

    // Draw.
    command->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
}

CubeApp::ComPtr<ID3D12Resource1> CubeApp::CreateTexture(const std::string& fileName)
{
    ComPtr<ID3D12Resource1> texture;
    int texWidth = 0, texHeight = 0, channels = 0;
    auto* pImage = stbi_load(fileName.c_str(), &texWidth, &texHeight, &channels, 0);
    assert(pImage);

    // サイズ、フォーマットからテクスチャリソースのDesc準備
    auto texDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        DXGI_FORMAT_R8G8B8A8_UNORM,
        texWidth, texHeight,
        1,  // 配列サイズ
        1   // ミップマップ数
    );

    // テクスチャ生成
    m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&texture)
    );

    // ステージングバッファ準備
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT layouts;
    UINT numRows;
    UINT64 rowSizeBytes, totalBytes;
    m_device->GetCopyableFootprints(&texDesc, 0, 1, 0, &layouts, &numRows, &rowSizeBytes, &totalBytes);
    ComPtr<ID3D12Resource> stagingBuffer = D3D12Util::CreateBuffer(m_device.Get(), static_cast<UINT>(totalBytes), nullptr);
    
    // ステージングバッファに画像をコピー
    {
        const UINT imagePitch = texWidth * sizeof(uint32_t);
        void* pBuf;
        CD3DX12_RANGE range(0, 0);
        stagingBuffer->Map(0, &range, &pBuf);
        for (UINT h = 0; h < numRows; ++h)
        {
            auto dst = static_cast<char*>(pBuf) + h * rowSizeBytes;
            auto src = pImage + h * imagePitch;
            memcpy(dst, src, imagePitch);
        }
    }

    // コマンド準備
    ComPtr<ID3D12GraphicsCommandList> command;
    m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_frameIndex].Get(), nullptr, IID_PPV_ARGS(&command));
    ComPtr<ID3D12Fence1> fence;
    m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

    // 転送コマンド
    D3D12_TEXTURE_COPY_LOCATION src{}, dst{};
    dst.pResource = texture.Get();
    dst.SubresourceIndex = 0;
    dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

    src.pResource = stagingBuffer.Get();
    src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    src.PlacedFootprint = layouts;
    command->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

    // コピー後はテクスチャとしてのステートへ
    auto barrierTex = CD3DX12_RESOURCE_BARRIER::Transition(
        texture.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
    command->ResourceBarrier(1, &barrierTex);

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

    stbi_image_free(pImage);
    return texture;
}