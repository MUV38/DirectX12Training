#include "MyApp.h"

#include <stdexcept>
#include <Texture/TextureLoader.h>
#include <D3D12/D3D12Util.h>
#include <Util/FullScreenQuad.h>

MyApp::MyApp()
{
	m_sceneParam.time = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	m_particleGlobalParam.velocityParam = DirectX::XMFLOAT4(0.5f, 0.0f, 0.0f, 0.0f);
}

MyApp::~MyApp()
{
}

void MyApp::OnInitialize()
{
	const auto& viewport = GetViewport();
	auto& descriptorManager = GetDescriptorManager();
	auto* device = GetDevice().Get();
	auto* commandAllocator = GetCommandAllocator().Get();
	auto* commandQueue = GetCommandQueue().Get();

	HRESULT hr;
	ComPtr<ID3DBlob> errBlob;

	// 頂点バッファ
	{
		const size_t bufferSize = sizeof(MyApp::Vertex) * NUM_PARTICLE;

		MyApp::Vertex* vertices = new MyApp::Vertex[NUM_PARTICLE];
		for (int i = 0; i < NUM_PARTICLE; ++i)
		{
			vertices[i].color = DirectX::XMFLOAT4(0, 1, 1, 1);
		}
		m_vertexBuffer = D3D12Util::CreateBuffer(device, bufferSize, vertices);
		delete[] vertices;

		// ビュー生成
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.SizeInBytes = bufferSize;
		m_vertexBufferView.StrideInBytes = sizeof(MyApp::Vertex);
	}

	// パーティクルデータ
	{
		const size_t bufferSize = sizeof(ParticleData) * NUM_PARTICLE;

		// resource
		ParticleData* datas = new ParticleData[NUM_PARTICLE];
		for (int i = 0; i < NUM_PARTICLE; ++i)
		{
			datas[i].position = DirectX::XMFLOAT3(
				static_cast<float>(i % 10) - 4.5f,
				static_cast<float>(i % 100 / 10) - 4.5f,
				static_cast<float>(i / 100)
			);
			datas[i].color = DirectX::XMFLOAT4(1, 1, 0, 1);
		}
		m_particleBuffer = D3D12Util::CreateUAVBuffer(device, bufferSize, datas);
		delete[] datas;

		// SRV
		m_particleSrv = descriptorManager.Alloc(DescriptorManager::DescriptorPoolType::CbvSrvUav);
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		{
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Buffer.NumElements = NUM_PARTICLE;
			srvDesc.Buffer.StructureByteStride = sizeof(ParticleData);
		}
		device->CreateShaderResourceView(m_particleBuffer.Get(), &srvDesc, m_particleSrv.GetCPUHandle());

		// UAV
		m_particleUav = descriptorManager.Alloc(DescriptorManager::DescriptorPoolType::CbvSrvUav);
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
		{
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.NumElements = NUM_PARTICLE;
			uavDesc.Buffer.StructureByteStride = sizeof(ParticleData);
		}
		device->CreateUnorderedAccessView(m_particleBuffer.Get(), nullptr, &uavDesc, m_particleUav.GetCPUHandle());
	}

	// シェーダーをコンパイル.
	m_vs.compile(L"VertexShader.hlsl", L"vs_6_0");
	m_gs.compile(L"GeometryShader.hlsl", L"gs_6_0");
	m_ps.compile(L"PixelShader.hlsl", L"ps_6_0");
	m_cs.compile(L"ComputeShader.hlsl", L"cs_6_0");

	// RootSignature.
	{
		// DescripterRange.
		CD3DX12_DESCRIPTOR_RANGE cbvScene, cbvModel, srv, uav, sampler;
		cbvScene.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		cbvModel.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
		srv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		uav.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		sampler.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);

		// RootParameter.
		CD3DX12_ROOT_PARAMETER rootParams[5];
		rootParams[0].InitAsDescriptorTable(1, &cbvScene, D3D12_SHADER_VISIBILITY_ALL);
		rootParams[1].InitAsDescriptorTable(1, &cbvModel, D3D12_SHADER_VISIBILITY_ALL);
		rootParams[2].InitAsDescriptorTable(1, &uav, D3D12_SHADER_VISIBILITY_VERTEX);
		rootParams[3].InitAsDescriptorTable(1, &srv, D3D12_SHADER_VISIBILITY_PIXEL);
		rootParams[4].InitAsDescriptorTable(1, &sampler, D3D12_SHADER_VISIBILITY_PIXEL);

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
	}

    // InputLayout.
    D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(MyApp::Vertex, color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA },
    };

	// BlendState
	CD3DX12_BLEND_DESC blendDesc(D3D12_DEFAULT);
	auto& blendRt0 = blendDesc.RenderTarget[0];
	{
		blendRt0.BlendEnable = true;
		blendRt0.BlendOp = D3D12_BLEND_OP_ADD;
		blendRt0.SrcBlend = D3D12_BLEND_ONE;
		blendRt0.DestBlend = D3D12_BLEND_ONE;

		blendRt0.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendRt0.SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
		blendRt0.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
	}
	
	// RasterizerState
	CD3DX12_RASTERIZER_DESC rasterizerState(D3D12_DEFAULT);
	rasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	// DepthStencilState
	CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

    // PipelineStateObject.
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
		{
			// Shader.
			psoDesc.VS = m_vs.getShaderByteCode();
			psoDesc.GS = m_gs.getShaderByteCode();
			psoDesc.PS = m_ps.getShaderByteCode();
			// BlendState.
			psoDesc.BlendState = blendDesc;
			// RasterizerState.
			psoDesc.RasterizerState = rasterizerState;
			// RenderTarget.
			psoDesc.NumRenderTargets = 1;
			psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			// DepthStencil.
			psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
			psoDesc.DepthStencilState = depthStencilDesc;
			// InputLayout.
			psoDesc.InputLayout = { inputElementDesc, _countof(inputElementDesc) };

			// RootSignature.
			psoDesc.pRootSignature = m_rootSignature.Get();
			psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
			// MultiSample.
			psoDesc.SampleDesc = { 1, 0 };
			psoDesc.SampleMask = UINT_MAX; // これを忘れると絵が出ない&警告も出ない.
		}
		hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipeline));
		if (FAILED(hr))
		{
			throw std::runtime_error("CreateGraphicsPipelineState failed");
		}
	}

    // ConstantBuffer/View.
	m_constantBuffers[CbScene].create(device, &descriptorManager, sizeof(SceneParameters));
	m_constantBuffers[CbModel].create(device, &descriptorManager, sizeof(ShaderParameters));
	m_constantBuffers[CbParticle].create(device, &descriptorManager, sizeof(ParticleGlobalParam));

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

	// テクスチャ読み込み
	{
		hr = TextureLoader::LoadDDS(
			device,
			ASSET_TEXTURE_ROOT"particle01.dds",
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

	// RootSignature(compute)
	{
		// DescripterRange.
		CD3DX12_DESCRIPTOR_RANGE uav, cbvScene, cbvParticle;
		uav.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		cbvScene.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		cbvParticle.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

		// RootParameter.
		CD3DX12_ROOT_PARAMETER rootParams[3];
		rootParams[0].InitAsDescriptorTable(1, &uav, D3D12_SHADER_VISIBILITY_ALL);
		rootParams[1].InitAsDescriptorTable(1, &cbvScene, D3D12_SHADER_VISIBILITY_ALL);
		rootParams[2].InitAsDescriptorTable(1, &cbvParticle, D3D12_SHADER_VISIBILITY_ALL);

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
			IID_PPV_ARGS(&m_computeRootSignature)
		);
		if (FAILED(hr))
		{
			throw std::runtime_error("CrateRootSignature failed.");
		}
	}
	// PSO(compute)
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
		{
			psoDesc.pRootSignature = m_computeRootSignature.Get();
			psoDesc.CS = m_cs.getShaderByteCode();
			psoDesc.NodeMask = 0;
			psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		}
		hr = device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_computePipeline));
		if (FAILED(hr))
		{
			throw std::runtime_error("CreateComputePipelineState failed");
		}

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

	// シェーダーパラメータ更新
	updateShaderParam(deltaTime);

	// コンスタントバッファ更新
	updateConstantBuffer(deltaTime);
}

void MyApp::OnRender(ComPtr<ID3D12GraphicsCommandList>& command)
{
	const auto& viewport = GetViewport();
	const auto& scissorRect = GetScissorRect();
	const auto frameIndex = GetFrameIndex();
	auto& descriptorManager = GetDescriptorManager();

	// compute particle
	{
		// PSO.
		command->SetPipelineState(m_computePipeline.Get());
		// RootSignature.
		command->SetComputeRootSignature(m_computeRootSignature.Get());
		// DescriptorHeap.
		DescriptorPool* cbvSrvUavDescriptorPool = descriptorManager.GetDescriptorPool(DescriptorManager::DescriptorPoolType::CbvSrvUav);
		ID3D12DescriptorHeap* heaps[] = {
			cbvSrvUavDescriptorPool->GetHeap()
		};
		command->SetDescriptorHeaps(_countof(heaps), heaps);
		// DescriptorTable.
		{
			command->SetComputeRootDescriptorTable(0, m_particleUav.GetGPUHandle());
			command->SetComputeRootDescriptorTable(1, m_constantBuffers[CbScene].getView(frameIndex));
			command->SetComputeRootDescriptorTable(2, m_constantBuffers[CbParticle].getView(frameIndex));
		}
		command->Dispatch(10, 1, 1);
	}

	// draw particle
	{
		auto& cbScene = m_constantBuffers[CbScene];
		auto& cbModel = m_constantBuffers[CbModel];

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

		// DescriptorTable.
		{
			command->SetGraphicsRootDescriptorTable(0, cbScene.getView(frameIndex));
			command->SetGraphicsRootDescriptorTable(1, cbModel.getView(frameIndex));
			command->SetGraphicsRootDescriptorTable(2, m_particleSrv.GetGPUHandle());
			command->SetGraphicsRootDescriptorTable(3, m_texture[TexAlbedo].GetShaderResourceView());
			command->SetGraphicsRootDescriptorTable(4, m_sampler.GetGPUHandle());
		}

		command->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		command->IASetVertexBuffers(0, 1, &m_vertexBufferView);
		command->DrawInstanced(NUM_PARTICLE, 1, 0, 0);
	}
}

// GUI更新
void MyApp::updateGUI(float deltaTime)
{
	if (ImGui::Begin("Shader Parameter"))
	{
		if (ImGui::TreeNode("Particle"))
		{
			if (ImGui::TreeNode("Velocity"))
			{
				auto& velocityParam = m_particleGlobalParam;
				ImGui::DragFloat("Scale", &m_particleGlobalParam.velocityParam.x, 0.01f, 0.0f, 1.0f);
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
		ImGui::End();
	}
}

// シェーダーパラメータ更新
void MyApp::updateShaderParam(float deltaTime)
{
	// SceneParameters
	{
		m_sceneParam.time.x = getElapsedTime();
		m_sceneParam.time.y = getDeltaTime();
	}
}

// コンスタントバッファ更新
void MyApp::updateConstantBuffer(float deltaTime)
{
	using namespace DirectX;

	const auto& viewport = GetViewport();
	const auto& scissorRect = GetScissorRect();
	const auto frameIndex = GetFrameIndex();
	auto& descriptorManager = GetDescriptorManager();

	// scene
	{
		auto& cbScene = m_constantBuffers[CbScene];

		void* p = nullptr;
		cbScene.map(frameIndex, &p);
		memcpy(p, &m_sceneParam, sizeof(SceneParameters));
		cbScene.unmap(frameIndex);
	}

	// model
	{
		auto& cbModel = m_constantBuffers[CbModel];

		// Matrix.
		ShaderParameters shaderParams;
		XMStoreFloat4x4(&shaderParams.mtxWorld, XMMatrixScaling(0.01f, 0.01f, 0.01f));
		auto mtxView = XMMatrixLookAtLH(
			XMVectorSet(0.0f, 0.0f, 5.0f, 0.f),
			XMVectorSet(0.f, 0.0f, 0.0f, 0.f),
			XMVectorSet(0.f, 1.f, 0.f, 0.f)
		);
		auto mtxInvView = XMMatrixInverse(nullptr, mtxView);
		auto mtxProj = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.f), viewport.Width / viewport.Height, 0.1f, 10000.0f);
		XMStoreFloat4x4(&shaderParams.mtxView, XMMatrixTranspose(mtxView));
		XMStoreFloat4x4(&shaderParams.mtxProj, XMMatrixTranspose(mtxProj));
		XMStoreFloat4x4(&shaderParams.mtxInvView, XMMatrixTranspose(mtxInvView));

		// Update constant buffer.
		{
			void* p = nullptr;
			cbModel.map(frameIndex, &p);
			memcpy(p, &shaderParams, sizeof(shaderParams));
			cbModel.unmap(frameIndex);
		}
	}

	// particle
	{
		auto& cbParticle = m_constantBuffers[CbParticle];

		void* p = nullptr;
		cbParticle.map(frameIndex, &p);
		memcpy(p, &m_particleGlobalParam, sizeof(ParticleGlobalParam));
		cbParticle.unmap(frameIndex);
	}
}
