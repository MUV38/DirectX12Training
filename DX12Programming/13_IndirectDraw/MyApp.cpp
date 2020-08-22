#include "MyApp.h"

#include <stdexcept>
#include "D3D12/D3D12Util.h"

MyApp::MyApp()
{

}

MyApp::~MyApp()
{

}

void MyApp::OnInitialize()
{
	const float k = 1.0f;
	const DirectX::XMFLOAT4 color(0.5f, 0.5f, 0.5f, 1.0f);

	Vertex cubeVertices[] = {
		// 前
		{ {-k,-k,-k}, {0, 0, -1}, color },
		{ {-k, k,-k}, {0, 0, -1}, color },
		{ { k, k,-k}, {0, 0, -1}, color },
		{ { k,-k,-k}, {0, 0, -1}, color },
		// 右
		{ { k,-k,-k}, {1, 0, 0}, color },
		{ { k, k,-k}, {1, 0, 0}, color },
		{ { k, k, k}, {1, 0, 0}, color },
		{ { k,-k, k}, {1, 0, 0}, color },
		// 左
		{ {-k,-k, k}, {-1, 0, 0}, color },
		{ {-k, k, k}, {-1, 0, 0}, color },
		{ {-k, k,-k}, {-1, 0, 0}, color },
		{ {-k,-k,-k}, {-1, 0, 0}, color },
		// 裏
		{ { k,-k, k}, {0, 0, 1}, color },
		{ { k, k, k}, {0, 0, 1}, color },
		{ {-k, k, k}, {0, 0, 1}, color },
		{ {-k,-k, k}, {0, 0, 1}, color },
		// 上
		{ {-k, k,-k}, {0, 1, 0}, color },
		{ {-k, k, k}, {0, 1, 0}, color },
		{ { k, k, k}, {0, 1, 0}, color },
		{ { k, k,-k}, {0, 1, 0}, color },
		// 底
		{ {-k,-k, k}, {0, -1, 0}, color },
		{ {-k,-k,-k}, {0, -1, 0}, color },
		{ { k,-k,-k}, {0, -1, 0}, color },
		{ { k,-k, k}, {0, -1, 0}, color },
	};
	uint32_t indices[] = {
	  0, 1, 2, 2, 3,0,
	  4, 5, 6, 6, 7,4,
	  8, 9, 10, 10, 11, 8,
	  12,13,14, 14,15,12,
	  16,17,18, 18,19,16,
	  20,21,22, 22,23,20,
	};

	ComPtr<ID3DBlob> errBlob;
	HRESULT hr;
	auto* device = GetDevice().Get();
	auto& descriptorManager = GetDescriptorManager();

	// 頂点バッファとインデックスバッファの生成
	mVertexBuffer = D3D12Util::CreateBuffer(device, sizeof(cubeVertices), cubeVertices);
	mIndexBuffer = D3D12Util::CreateBuffer(device, sizeof(indices), indices);
	mIndexCount = _countof(indices);

	// 頂点、インデックスバッファビュー設定
	mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
	mVertexBufferView.SizeInBytes = sizeof(cubeVertices);
	mVertexBufferView.StrideInBytes = sizeof(Vertex);
	mIndexBufferView.BufferLocation = mIndexBuffer->GetGPUVirtualAddress();
	mIndexBufferView.SizeInBytes = sizeof(indices);
	mIndexBufferView.Format = DXGI_FORMAT_R32_UINT;

	// シェーダーをコンパイル
	mVS.compile(L"VertexShader.hlsl", L"vs_6_0");
	mPS.compile(L"PixelShader.hlsl", L"ps_6_0");

	// DescripterRange.
	CD3DX12_DESCRIPTOR_RANGE cbv;
	cbv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

	// RootParameter.
	CD3DX12_ROOT_PARAMETER rootParams[1];
	rootParams[0].InitAsDescriptorTable(1, &cbv, D3D12_SHADER_VISIBILITY_VERTEX);

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
		IID_PPV_ARGS(&mRootSignature)
	);
	if (FAILED(hr))
	{
		throw std::runtime_error("CrateRootSignature failed.");
	}

	// InputLayout.
	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, pos), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA },
	};

	// PipelineStateObject.
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	{
		// Shader.
		psoDesc.VS = mVS.getShaderByteCode();
		psoDesc.PS = mPS.getShaderByteCode();
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
		psoDesc.pRootSignature = mRootSignature.Get();
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		// MultiSample.
		psoDesc.SampleDesc = { 1, 0 };
		psoDesc.SampleMask = UINT_MAX; // これを忘れると絵が出ない&警告も出ない.
	}
	hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPipeline));
	if (FAILED(hr))
	{
		throw std::runtime_error("CreateGraphicsPipelineState failed");
	}

	// ConstantBuffer/View.
	{
		mConstantBuffers[CbModel].create(device, &descriptorManager, sizeof(ShaderParameters));
	}

}
void MyApp::OnFinalize()
{
	WaitForGPU();
}
void MyApp::OnUpdate(float deltaTime)
{
	using namespace DirectX;

	const auto frameIndex = GetFrameIndex();
	const auto& viewport = GetViewport();
	const auto& scissorRect = GetScissorRect();
	auto& descriptorManager = GetDescriptorManager();

	auto& cbModel = mConstantBuffers[CbModel];

	// Matrix.
	ShaderParameters shaderParams;
	XMStoreFloat4x4(&shaderParams.mtxWorld, XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(45.f)));
	auto mtxView = XMMatrixLookAtLH(
		XMVectorSet(0.f, 3.f, -5.f, 0.f),
		XMVectorSet(0.f, 0.f, 0.f, 0.f),
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
}
void MyApp::OnRender(ComPtr<ID3D12GraphicsCommandList>& command)
{
	const auto frameIndex = GetFrameIndex();
	const auto& viewport = GetViewport();
	const auto& scissorRect = GetScissorRect();
	auto& descriptorManager = GetDescriptorManager();
	auto& cbModel = mConstantBuffers[CbModel];

	// PSO.
	command->SetPipelineState(mPipeline.Get());
	// RootSignature.
	command->SetGraphicsRootSignature(mRootSignature.Get());
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
	command->IASetVertexBuffers(0, 1, &mVertexBufferView);
	command->IASetIndexBuffer(&mIndexBufferView);

	command->SetGraphicsRootDescriptorTable(0, cbModel.getView(frameIndex));

	// Draw.
	command->DrawIndexedInstanced(mIndexCount, 1, 0, 0, 0);
}
