#include "App.h"

#undef min
#undef max

#include <stdexcept>
#include <Util/D3D12Util.h>

App::App()
{
}

App::~App()
{
}

void App::OnInitialize()
{
    auto* device = GetDevice().Get();
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
}

void App::OnFinalize()
{
    WaitForGPU();

#if 0
    auto index = m_swapChain->GetCurrentBackBufferIndex();
    auto fence = m_frameFences[index];
    auto value = ++m_frameFenceValues[index];
    m_commandQueue->Signal(fence.Get(), value);
    fence->SetEventOnCompletion(value, m_fenceWaitEvent);
    WaitForSingleObject(m_fenceWaitEvent, GpuWaitTimeout);
#endif
}

void App::OnRender(ComPtr<ID3D12GraphicsCommandList>& command)
{
    using namespace DirectX;

    const auto& viewport = GetViewport();
    const auto& scissorRect = GetScissorRect();
    const auto frameIndex = GetFrameIndex();
    auto& descriptorManager = GetDescriptorManager();

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

    // PSO.
    command->SetPipelineState(m_pipeline.Get());
    // RootSignature.
    command->SetGraphicsRootSignature(m_rootSignature.Get());
    // Viewport, Scissor.
    command->RSSetViewports(1, &viewport);
    command->RSSetScissorRects(1, &scissorRect);

    // DescriptorHeap.
	DescriptorPool* cbvSrvUavDescriptorPool = descriptorManager.GetDescriptorPool(DescriptorManager::DescriptorPoolType::CbvSrvUav);
    ID3D12DescriptorHeap* heaps[] = {
		cbvSrvUavDescriptorPool->GetHeap()
    };
    command->SetDescriptorHeaps(_countof(heaps), heaps);

    // DescriptorTable.
	int cbvModelIndex = CbvModel * frameIndex + frameIndex;
    command->SetGraphicsRootDescriptorTable(0, m_cbViews[cbvModelIndex].GetGPUHandle());
    
    // DrawModel
    m_modelLoader.Draw(command.Get());
}