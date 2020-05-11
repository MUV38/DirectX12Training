#include "MyApp.h"
#include <stdexcept>
#define STB_IMAGE_IMPLEMENTATION
#include <ThirdParty/stb/stb_image.h>
#include <D3D12/D3D12Util.h>

MyApp::MyApp()
{
	mGrassParam.BottomColor = DirectX::XMFLOAT4(0.001f, 0.122f, 0.0f, 1.0f);
	mGrassParam.TopColor = DirectX::XMFLOAT4(0.0f, 1.0f, 0.226f, 1.0f);
	mGrassParam.HeightParam = DirectX::XMFLOAT4(0.3f, 0.3f, 0.4f, 0.5f);
	mGrassParam.WidthParam = DirectX::XMFLOAT4(0.01f, 2.0f, 1.0f, 0.75f);
	mGrassParam.WindParam = DirectX::XMFLOAT4(0.01f, 0.05f, 0.0f, 0.0f);
	mGrassParam.WindParam2 = DirectX::XMFLOAT4(0.3f, 1.0f, 2.0f, 0.0f);
	mGrassParam.Bend = DirectX::XMFLOAT4(1.0f, 1.0f, 2.0f, 0.0f);
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

	const int subdivision = 16;
	const float k = 1.0f;
	const DirectX::XMFLOAT3 normal(0.0f, 1.0f, 0.0f);
	const DirectX::XMFLOAT4 white(1.f, 1.f, 1.f, 1.f);

	Vertex vertices[subdivision * subdivision * 3 * 2];
	uint32_t indices[subdivision * subdivision * 3 * 2];
	float vertexOffset = k / static_cast<float>(subdivision);
	float uvOffset = 1.0f / static_cast<float>(subdivision);
	for (int h = 0; h < subdivision; ++h)
	{
		int heightIndex = h * subdivision * 3 * 2;
		for (int w = 0; w < subdivision; ++w)
		{
			int widthIndex = w * 3 * 2;

			DirectX::XMFLOAT3 leftTopPos(
				-vertexOffset * (static_cast<float>(subdivision) * 0.5f) + vertexOffset * static_cast<float>(w),
				0.0f,
				-vertexOffset * (static_cast<float>(subdivision) * 0.5f) + vertexOffset * static_cast<float>(h)
			);

			DirectX::XMFLOAT2 leftTopUV(
				uvOffset * static_cast<float>(w),
				uvOffset * static_cast<float>(h)
			);

			vertices[heightIndex + widthIndex + 0].pos = leftTopPos;
			vertices[heightIndex + widthIndex + 1].pos = DirectX::XMFLOAT3(leftTopPos.x + vertexOffset, leftTopPos.y, leftTopPos.z);
			vertices[heightIndex + widthIndex + 2].pos = DirectX::XMFLOAT3(leftTopPos.x,                leftTopPos.y, leftTopPos.z + vertexOffset);
			vertices[heightIndex + widthIndex + 3].pos = DirectX::XMFLOAT3(leftTopPos.x + vertexOffset, leftTopPos.y, leftTopPos.z);
			vertices[heightIndex + widthIndex + 4].pos = DirectX::XMFLOAT3(leftTopPos.x + vertexOffset, leftTopPos.y, leftTopPos.z + vertexOffset);
			vertices[heightIndex + widthIndex + 5].pos = DirectX::XMFLOAT3(leftTopPos.x,                leftTopPos.y, leftTopPos.z + vertexOffset);
		
			vertices[heightIndex + widthIndex + 0].uv = leftTopUV;
			vertices[heightIndex + widthIndex + 1].uv = DirectX::XMFLOAT2(leftTopUV.x + uvOffset, leftTopUV.y);
			vertices[heightIndex + widthIndex + 2].uv = DirectX::XMFLOAT2(leftTopUV.x,            leftTopUV.y + uvOffset);
			vertices[heightIndex + widthIndex + 3].uv = DirectX::XMFLOAT2(leftTopUV.x + uvOffset, leftTopUV.y);
			vertices[heightIndex + widthIndex + 4].uv = DirectX::XMFLOAT2(leftTopUV.x + uvOffset, leftTopUV.y + uvOffset);
			vertices[heightIndex + widthIndex + 5].uv = DirectX::XMFLOAT2(leftTopUV.x,            leftTopUV.y + uvOffset);

			for (int i = 0; i < 6; ++i)
			{
				vertices[heightIndex + widthIndex + i].color = white;
				vertices[heightIndex + widthIndex + i].normal = normal;

				indices[heightIndex + widthIndex + i] = heightIndex + widthIndex + i;
			}
		}
	}

	// 頂点バッファとインデックスバッファの生成.
	m_vertexBuffer = D3D12Util::CreateBuffer(device, sizeof(vertices), vertices);
	m_indexBuffer = D3D12Util::CreateBuffer(device, sizeof(indices), indices);
	m_indexCount = _countof(indices);

	// 各バッファのビューを生成.
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = sizeof(vertices);
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);
	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.SizeInBytes = sizeof(indices);
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	// シェーダーをコンパイル.
	m_vs.compile(L"VertexShader.hlsl", L"vs_6_0");
	m_gs.compile(L"GeometryShader.hlsl", L"gs_6_0");
	m_ps.compile(L"PixelShader.hlsl", L"ps_6_0");

	// DescripterRange.
	CD3DX12_DESCRIPTOR_RANGE cbvModel, cbvGrass;
	cbvModel.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	cbvGrass.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	// RootParameter.
	CD3DX12_ROOT_PARAMETER rootParams[2];
	rootParams[0].InitAsDescriptorTable(1, &cbvModel, D3D12_SHADER_VISIBILITY_ALL);
	rootParams[1].InitAsDescriptorTable(1, &cbvGrass, D3D12_SHADER_VISIBILITY_ALL);

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

	// RasterizerState.
	CD3DX12_RASTERIZER_DESC rasterizerDesc(D3D12_DEFAULT);
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE; // 両面
	//rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;

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
		psoDesc.RasterizerState = rasterizerDesc;
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
		m_constantBuffers[CbGrass].create(device, &descriptorManager, sizeof(GrassParameters));
	}
}

void MyApp::OnFinalize()
{
	WaitForGPU();
}

void MyApp::OnUpdate(float deltaTime)
{
	// GUI更新
	updateGUI(deltaTime);

	// コンスタントバッファ更新
	updateConstantBuffer(deltaTime);
}

void MyApp::OnRender(ComPtr<ID3D12GraphicsCommandList>& command)
{
	auto frameIndex = GetFrameIndex();
	const auto& viewport = GetViewport();
	const auto& scissorRect = GetScissorRect();
	auto& descriptorManager = GetDescriptorManager();
	auto& cbModel = m_constantBuffers[CbModel];
	auto& cbGrass = m_constantBuffers[CbGrass];

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
	command->SetGraphicsRootDescriptorTable(1, cbGrass.getView(frameIndex));

	// Draw.
	command->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
}

//GUI更新
void MyApp::updateGUI(float deltaTime)
{
	if (ImGui::Begin("GrassParameter", nullptr))
	{
		ImGui::SetNextWindowSize(ImVec2(300, 300));
		if (ImGui::TreeNode("Color"))
		{
			DirectX::XMFLOAT4* bottomPtr = &mGrassParam.BottomColor;
			DirectX::XMFLOAT4* topPtr = &mGrassParam.TopColor;

			float bottom[3]{ bottomPtr->x, bottomPtr->y, bottomPtr->z };
			float top[3]{ topPtr->x, topPtr->y, topPtr->z };
			if (ImGui::ColorPicker3("ButtomColor", bottom))
			{
				bottomPtr->x = bottom[0];
				bottomPtr->y = bottom[1];
				bottomPtr->z = bottom[2];
				bottomPtr->w = 0.0f;
			}
			if (ImGui::ColorPicker3("TopColor", top))
			{
				topPtr->x = top[0];
				topPtr->y = top[1];
				topPtr->z = top[2];
				topPtr->w = 0.0f;
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Height"))
		{
			DirectX::XMFLOAT4* heightPtr = &mGrassParam.HeightParam;
			float height{ heightPtr->x }, bottom{ heightPtr->y }, middle{ heightPtr->z }, top{ heightPtr->w };
			if (ImGui::DragFloat("Height", &height, 0.1f, 0.0f, 20.0f))
			{
				heightPtr->x = height;
			}
			if (ImGui::DragFloat("Bottom", &bottom, 0.01f, 0.0f, 5.0f))
			{
				heightPtr->y = bottom;
			}
			if (ImGui::DragFloat("Middle", &middle, 0.01f, 0.0f, 5.0f))
			{
				heightPtr->z = middle;
			}
			if (ImGui::DragFloat("Top", &top, 0.01f, 0.0f, 5.0f))
			{
				heightPtr->w = top;
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Width"))
		{
			DirectX::XMFLOAT4* widthPtr = &mGrassParam.WidthParam;
			float width{ widthPtr->x }, bottom{ widthPtr->y }, middle{ widthPtr->z }, top{ widthPtr->w };
			if (ImGui::DragFloat("Width", &width, 0.1f, 0.0f, 20.0f))
			{
				widthPtr->x = width;
			}
			if (ImGui::DragFloat("Bottom", &bottom, 0.01f, 0.0f, 5.0f))
			{
				widthPtr->y = bottom;
			}
			if (ImGui::DragFloat("Middle", &middle, 0.01f, 0.0f, 5.0f))
			{
				widthPtr->z = middle;
			}
			if (ImGui::DragFloat("Top", &top, 0.01f, 0.0f, 5.0f))
			{
				widthPtr->w = top;
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Wind"))
		{
			DirectX::XMFLOAT4* windPtr = &mGrassParam.WindParam;
			float power{ windPtr->x }, frequency{ windPtr->y };
			if (ImGui::DragFloat("Power", &power, 0.1f, 0.0f, 10.0f))
			{
				windPtr->x = power;
			}
			if (ImGui::DragFloat("Frequency", &frequency, 0.001f, 0.0f, 0.1f))
			{
				windPtr->y = frequency;
			}
			if (ImGui::TreeNode("PowerRate"))
			{
				DirectX::XMFLOAT4* wind2Ptr = &mGrassParam.WindParam2;
				float bottom{ wind2Ptr->x }, middle{ wind2Ptr->y }, top{ wind2Ptr->z };
				if (ImGui::DragFloat("Bottom", &bottom, 0.01f, 0.0f, 5.0f))
				{
					wind2Ptr->x = bottom;
				}
				if (ImGui::DragFloat("Middle", &middle, 0.01f, 0.0f, 5.0f))
				{
					wind2Ptr->y = middle;
				}
				if (ImGui::DragFloat("Top", &top, 0.01f, 0.0f, 5.0f))
				{
					wind2Ptr->z = top;
				}
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Bend"))
		{
			DirectX::XMFLOAT4* bendPtr = &mGrassParam.Bend;
			float bottom{ bendPtr->x }, middle{ bendPtr->y }, top{ bendPtr->z };
			if (ImGui::DragFloat("Bottom", &bottom, 0.01f, 0.0f, 5.0f))
			{
				bendPtr->x = bottom;
			}
			if (ImGui::DragFloat("Middle", &middle, 0.01f, 0.0f, 5.0f))
			{
				bendPtr->y = middle;
			}
			if (ImGui::DragFloat("Top", &top, 0.01f, 0.0f, 5.0f))
			{
				bendPtr->z = top;
			}
			ImGui::TreePop();
		}
		ImGui::End();
	}
}

// コンスタントバッファ更新
void MyApp::updateConstantBuffer(float deltaTime)
{
	using namespace DirectX;

	auto frameIndex = GetFrameIndex();
	const auto& viewport = GetViewport();

	// model
	{
		auto& cbModel = m_constantBuffers[CbModel];

		// Matrix.
		ShaderParameters shaderParams;

		XMMATRIX mtxWorld = XMMatrixScaling(256.0f, 256.0f, 256.0f);
		mtxWorld *= XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(-90.0f));
		mtxWorld *= XMMatrixTranslation(0.0f, 0.0f, 0.0f);
		XMStoreFloat4x4(&shaderParams.mtxWorld, mtxWorld);

		auto mtxView = XMMatrixLookAtLH(
			XMVectorSet(0.f, 300.0f, -800.0f, 0.f),
			XMVectorSet(0.f, 100.0f, 0.f, 0.f),
			XMVectorSet(0.f, 1.f, 0.f, 0.f)
		);
		auto mtxProj = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.f), viewport.Width / viewport.Height, 0.1f, 10000.0f);
		XMStoreFloat4x4(&shaderParams.mtxView, XMMatrixTranspose(mtxView));
		XMStoreFloat4x4(&shaderParams.mtxProj, XMMatrixTranspose(mtxProj));

		shaderParams.time = DirectX::XMFLOAT4(getElapsedTime(), deltaTime, 0.0f, 0.0f);

		// Update constant buffer.
		{
			void* p = nullptr;
			cbModel.map(frameIndex, &p);
			memcpy(p, &shaderParams, sizeof(shaderParams));
			cbModel.unmap(frameIndex);
		}
	}

	// grass
	{
		auto& cbGrass = m_constantBuffers[CbGrass];
		// Update constant buffer.
		{
			void* p = nullptr;
			cbGrass.map(frameIndex, &p);
			memcpy(p, &mGrassParam, sizeof(GrassParameters));
			cbGrass.unmap(frameIndex);
		}
	}
}
