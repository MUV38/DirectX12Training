#include "MyApp.h"

#undef min
#undef max

#include <stdexcept>
#include <Texture/TextureLoader.h>
#include <D3D12/D3D12Util.h>

MyApp::MyApp()
{
}

MyApp::~MyApp()
{
}

void MyApp::OnInitialize()
{
    auto& descriptorManager = GetDescriptorManager();
    auto* device = GetDevice().Get();
    auto* commandAllocator = GetCommandAllocator().Get();
    auto* commandQueue = GetCommandQueue().Get();

    // モデル読み込み
    m_modelLoader.Load(device, ASSET_MODEL_ROOT"sponza/sponza.fbx");

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
			ASSET_TEXTURE_ROOT_W"uvChecker.dds",
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
    const auto frameIndex = GetFrameIndex();
    auto& descriptorManager = GetDescriptorManager();

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
        D3D12_RANGE range{ 0, 0 };
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

    // DescriptorTable.
    {
        command->SetGraphicsRootDescriptorTable(0, cbModel.getView(frameIndex));
        command->SetGraphicsRootDescriptorTable(1, m_texture.GetShaderResourceView());
        command->SetGraphicsRootDescriptorTable(2, m_sampler.GetGPUHandle());
    }

    // DrawModel
    m_modelLoader.Draw(command.Get());
}