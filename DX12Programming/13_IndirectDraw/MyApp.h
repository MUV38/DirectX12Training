#pragma once

#include "Application/Application.h"
#include "Shader/ShaderObject.h"
#include "ConstantBuffer/ConstantBuffer.h"

class MyApp : public Application
{
public:
    MyApp();
    virtual ~MyApp();

protected:
    virtual void OnInitialize() override;
    virtual void OnFinalize() override;
    virtual void OnUpdate(float deltaTime) override;
    virtual void OnRender(ComPtr<ID3D12GraphicsCommandList>& command) override;

private:
    struct Vertex
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT4 color;
    };

    struct ShaderParameters
    {
        DirectX::XMFLOAT4X4 mtxWorld;
        DirectX::XMFLOAT4X4 mtxView;
        DirectX::XMFLOAT4X4 mtxProj;
    };

    enum
    {
        CbModel,
        CbNum
    };

private:
    ComPtr<ID3D12Resource> mVertexBuffer;
    ComPtr<ID3D12Resource> mIndexBuffer;
    D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
    D3D12_INDEX_BUFFER_VIEW mIndexBufferView;
    UINT mIndexCount;

    ShaderObject mVS;
    ShaderObject mPS;

    ComPtr<ID3D12RootSignature> mRootSignature;
    ComPtr<ID3D12PipelineState> mPipeline;

    ConstantBuffer mConstantBuffers[CbNum];
};