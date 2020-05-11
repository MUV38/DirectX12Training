#pragma once

#include <Application/Application.h>
#include <DirectXMath.h>
#include <ConstantBuffer/ConstantBuffer.h>
#include <Shader/ShaderObject.h>

class MyApp : public Application
{
public:
    MyApp();
    virtual ~MyApp();

public:
    struct Vertex
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT4 color;
        DirectX::XMFLOAT2 uv;
        DirectX::XMFLOAT3 normal;
    };

    struct ShaderParameters
    {
        DirectX::XMFLOAT4X4 mtxWorld;
        DirectX::XMFLOAT4X4 mtxView;
        DirectX::XMFLOAT4X4 mtxProj;
    };

    struct GrassParameters
    {
        DirectX::XMFLOAT4 BottomColor; // rgb:color a:N/A
        DirectX::XMFLOAT4 TopColor; // rgb:color a:N/A
        DirectX::XMFLOAT4 HeightParam; // r:height g:bottomRate b:middleRate a:topRate
        DirectX::XMFLOAT4 WidthParam; // r:width g:bottomRate b:middleRate a:topRate
        DirectX::XMFLOAT4 WindParam; // r:power g:frequency b:N/A a:N/A
        DirectX::XMFLOAT4 WindParam2; // r:powerRateBottom g:powerRateMiddle b:powerRateTop a:N/A
    };

	enum
	{
		CbModel,
        CbGrass,
		CbNum
	};

public:
    virtual void OnInitialize() override;
    virtual void OnFinalize() override;
    virtual void OnUpdate(float deltaTime) override;
    virtual void OnRender(ComPtr<ID3D12GraphicsCommandList>& command) override;

private:
    // GUI更新
    void updateGUI(float deltaTime);
    // コンスタントバッファ更新
    void updateConstantBuffer(float deltaTime);

private:    
    ComPtr<ID3D12Resource> m_vertexBuffer;
    ComPtr<ID3D12Resource> m_indexBuffer;
    ComPtr<ID3D12Resource> m_texture;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
    UINT m_indexCount;

    ShaderObject m_vs, m_gs, m_ps;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12PipelineState> m_pipeline;
    ConstantBuffer m_constantBuffers[CbNum];

    GrassParameters mGrassParam;
};
