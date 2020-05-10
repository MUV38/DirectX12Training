#include "MyApp.h"
#include <stdexcept>
#define STB_IMAGE_IMPLEMENTATION
#include <ThirdParty/stb/stb_image.h>
#include <D3D12/D3D12Util.h>

MyApp::MyApp()
{
}

MyApp ::~MyApp()
{
}

void MyApp::OnInitialize()
{
	auto* device = GetDevice().Get();
	auto& descriptorManager = GetDescriptorManager();

	HRESULT hr;
	ComPtr<ID3DBlob> errBlob;

	const float k = 1.0f;
	const DirectX::XMFLOAT4 white(1.f, 1.f, 1.f, 1.f);

	Vertex cubeVertices[] = {
		{ {-k, 0.0f,-k}, white, { 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f} },
		{ {-k, 0.0f, k}, white, { 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} },
		{ { k, 0.0f, k}, white, { 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f} },
		{ { k, 0.0f,-k}, white, { 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f} },
	};
	uint32_t indices[] = {
	  0, 1, 2, 2, 3,0,
	};

	// 頂点バッファとインデックスバッファの生成.
	m_vertexBuffer = D3D12Util::CreateBuffer(device, sizeof(cubeVertices), cubeVertices);
	m_indexBuffer = D3D12Util::CreateBuffer(device, sizeof(indices), indices);
	m_indexCount = _countof(indices);

	// 各バッファのビューを生成.
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = sizeof(cubeVertices);
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);
	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.SizeInBytes = sizeof(indices);
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	// シェーダーをコンパイル.
	m_vs.compile(L"VertexShader.hlsl", L"vs_6_0");
	m_gs.compile(L"GeometryShader.hlsl", L"gs_6_0");
	m_ps.compile(L"PixelShader.hlsl", L"ps_6_0");

	// DescripterRange.
	CD3DX12_DESCRIPTOR_RANGE cbv, srv, sampler;
	cbv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	srv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	sampler.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);

	// RootParameter.
	CD3DX12_ROOT_PARAMETER rootParams[1];
	rootParams[0].InitAsDescriptorTable(1, &cbv, D3D12_SHADER_VISIBILITY_ALL);

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
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, uv), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA },
	};

	// PipelineStateObject.
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	{
		// Shader.
		psoDesc.VS = m_vs.getShaderByteCode();
		psoDesc.GS = m_gs.getShaderByteCode();
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
		psoDesc.SampleMask = UINT_MAX; // これを忘れると絵が出ない&警告も出ない.
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
}

void MyApp::OnFinalize()
{
	WaitForGPU();
}

void MyApp::OnRender(ComPtr<ID3D12GraphicsCommandList>& command)
{
    using namespace DirectX;

	auto frameIndex = GetFrameIndex();
	const auto& viewport = GetViewport();
	const auto& scissorRect = GetScissorRect();
	auto& descriptorManager = GetDescriptorManager();

	auto& cbModel = m_constantBuffers[CbModel];

    // Matrix.
    ShaderParameters shaderParams;
    XMStoreFloat4x4(&shaderParams.mtxWorld, XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(0.0f)));
    auto mtxView = XMMatrixLookAtLH(
        XMVectorSet(0.f, 3.f, -10.f, 0.f),
        XMVectorSet(0.f, 2.f,  0.f, 0.f),
        XMVectorSet(0.f, 1.f,  0.f, 0.f)
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

    // PSO.
    command->SetPipelineState(m_pipeline.Get());
    // RootSignature.
    command->SetGraphicsRootSignature(m_rootSignature.Get());
    // Viewport, Scissor.
    command->RSSetViewports(1, &viewport);
    command->RSSetScissorRects(1, &scissorRect);

    // DescriptorHeap.
	DescriptorPool* cbvSrvUavDescriptorPool = descriptorManager.GetDescriptorPool(DescriptorManager::DescriptorPoolType::CbvSrvUav);
	DescriptorPool* samplerDescriptorPool = descriptorManager.GetDescriptorPool(DescriptorManager::DescriptorPoolType::Sampler);
    ID3D12DescriptorHeap* heaps[] = {
		cbvSrvUavDescriptorPool->GetHeap(), samplerDescriptorPool->GetHeap()
    };
    command->SetDescriptorHeaps(_countof(heaps), heaps);

    // PrimitiveType, Vertex, Index.
    command->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    command->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    command->IASetIndexBuffer(&m_indexBufferView);

    command->SetGraphicsRootDescriptorTable(0, cbModel.getView(frameIndex));

    // Draw.
    command->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
}