#include "MyApp.h"
#include <stdexcept>
#include <D3D12/D3D12Util.h>

MyApp::MyApp()
{
    m_tessParam.TessFactor = DirectX::XMFLOAT4(3.0f, 0.0f, 0.0f, 0.0f);
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

    Vertex triangleVertices[] = {
        { {  0.0f, 0.25f, 0.5f }, { 1.0f, 0.0f,0.0f,1.0f} },
        { { 0.25f,-0.25f, 0.5f }, { 0.0f, 1.0f,0.0f,1.0f} },
        { {-0.25f,-0.25f, 0.5f }, { 0.0f, 0.0f,1.0f,1.0f} },
    };
    uint32_t indices[] = { 0, 1, 2 };

    // 頂点バッファとインデックスバッファの生成.
    m_vertexBuffer = CreateBuffer(sizeof(triangleVertices), triangleVertices);
    m_indexBuffer = CreateBuffer(sizeof(indices), indices);
    m_indexCount = _countof(indices);

    // 各バッファのビューを生成.
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = sizeof(triangleVertices);
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);
    m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.SizeInBytes = sizeof(indices);
    m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;

    // シェーダーをコンパイル.
    m_vs.compile(L"VertexShader.hlsl", L"vs_6_0");
    m_hs.compile(L"HullShader.hlsl", L"hs_6_0");
    m_ds.compile(L"DomainShader.hlsl", L"ds_6_0");
    m_ps.compile(L"PixelShader.hlsl", L"ps_6_0");

    CD3DX12_DESCRIPTOR_RANGE cbvTess;
    cbvTess.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

    CD3DX12_ROOT_PARAMETER rootParams[1];
    rootParams[0].InitAsDescriptorTable(1, &cbvTess, D3D12_SHADER_VISIBILITY_ALL);

    // RootSignatureの構築
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc = {};
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
    // RootSignatureの生成.
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
    };

    // RasterizeState
    CD3DX12_RASTERIZER_DESC rasterizerDesc(D3D12_DEFAULT);
    rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;

    // PipelineStateObject.
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    {
        // Shader.
        psoDesc.VS = m_vs.getShaderByteCode();
        psoDesc.HS = m_hs.getShaderByteCode();
        psoDesc.DS = m_ds.getShaderByteCode();
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
        psoDesc.InputLayout = { inputElementDesc, _countof(inputElementDesc) };
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        // RootSignature.
        psoDesc.pRootSignature = m_rootSignature.Get();
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
        // MultiSample.
        psoDesc.SampleDesc = { 1, 0 };
        psoDesc.SampleMask = UINT_MAX; // これを忘れると絵が出ない&警告も出ない.
    }
    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipeline));
    if (FAILED(hr))
    {
        throw std::runtime_error("CreateGraphicsPipelineState failed");
    }

    // ConstantBuffer
    {
        m_constantBuffers[CbTessellation].create(device, &descriptorManager, sizeof(TessellationParameters));
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
    const auto& viewport = GetViewport();
    const auto& scissorRect = GetScissorRect();
    auto& descriptorManager = GetDescriptorManager();
    const auto frameIndex = GetFrameIndex();

    // PSO.
    command->SetPipelineState(m_pipeline.Get());
    // RootSignature.
    command->SetGraphicsRootSignature(m_rootSignature.Get());
    // DescriptorHeap.
    DescriptorPool* cbvSrvUavDescriptorPool = descriptorManager.GetDescriptorPool(DescriptorManager::DescriptorPoolType::CbvSrvUav);
    ID3D12DescriptorHeap* heaps[] = {
        cbvSrvUavDescriptorPool->GetHeap()
    };
    command->SetDescriptorHeaps(_countof(heaps), heaps);
    // DescriptorTable.
    command->SetGraphicsRootDescriptorTable(0, m_constantBuffers[CbTessellation].getView(frameIndex));

    // Viewport, Scissor.
    command->RSSetViewports(1, &viewport);
    command->RSSetScissorRects(1, &scissorRect);

    // PrimitiveType, Vertex, Index.
    command->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
    command->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    command->IASetIndexBuffer(&m_indexBufferView);

    // Draw.
    command->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
}

MyApp::ComPtr<ID3D12Resource1> MyApp::CreateBuffer(UINT bufferSize, const void* initialData)
{
    auto* device = GetDevice().Get();

    HRESULT hr;
    ComPtr<ID3D12Resource1> buffer;
    hr = device->CreateCommittedResource(
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

// GUI更新
void MyApp::updateGUI(float deltaTime)
{
    if (ImGui::Begin("ShaderParameters"))
    {
        if (ImGui::TreeNode("Tessellation"))
        {
            auto* tessFactor = &m_tessParam.TessFactor;
            if (ImGui::DragFloat("Subdivision", &tessFactor->x, 1.0f, 1.0f, 32.0f))
            {
            }
            ImGui::TreePop();
        }

        ImGui::End();
    }
}

// コンスタントバッファ更新
void MyApp::updateConstantBuffer(float deltaTime)
{
    const auto frameIndex = GetFrameIndex();

    // CbTessellation
    {
        auto& cbTess = m_constantBuffers[CbTessellation];

        void* p = nullptr;
        cbTess.map(frameIndex, &p);
        memcpy(p, &m_tessParam, sizeof(TessellationParameters));
        cbTess.unmap(frameIndex);
    }
}