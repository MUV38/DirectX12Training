#pragma once

#include "D3D12AppBase.h"
#include <DirectXMath.h>

class CubeApp : public D3D12AppBase
{
public:
    CubeApp();
    virtual ~CubeApp();

public:
    struct Vertex
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT4 color;
        DirectX::XMFLOAT2 uv;
    };

    struct ShaderParameters
    {
        DirectX::XMFLOAT4X4 mtxWorld;
        DirectX::XMFLOAT4X4 mtxView;
        DirectX::XMFLOAT4X4 mtxProj;
    };

    enum
    {
        TextureSrvDescriptorBase = 0,
        ConstantBufferDescriptorBase = 1,

        SamplerDescriptorBase = 0,
    };

public:
    virtual void Prepare() override;
    virtual void Cleanup() override;
    virtual void MakeCommand(ComPtr<ID3D12GraphicsCommandList>& command) override;

private:
    /// バッファの作成
    ComPtr<ID3D12Resource1> CreateBuffer(UINT bufferSize, const void* initialData);

    /// テクスチャの作成
    ComPtr<ID3D12Resource1> CreateTexture(const std::string& fileName);

    /// ディスクリプタヒープのセットアップ
    void PrepareDescriptorHeapForCubeApp();

private:
    ComPtr<ID3D12DescriptorHeap> m_heapSrvCbv;
    ComPtr<ID3D12DescriptorHeap> m_heapSampler;
    UINT m_samplerDescriptorSize;
    
    ComPtr<ID3D12Resource1> m_vertexBuffer;
    ComPtr<ID3D12Resource1> m_indexBuffer;
    ComPtr<ID3D12Resource1> m_texture;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
    UINT m_indexCount;

    ComPtr<ID3DBlob> m_vs, m_ps;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12PipelineState> m_pipeline;
    std::vector < ComPtr<ID3D12Resource1>> m_constantBuffers;

    D3D12_GPU_DESCRIPTOR_HANDLE m_sampler;
    D3D12_GPU_DESCRIPTOR_HANDLE m_srv;
    std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_cbViews;
};
