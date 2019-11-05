#include "pch.h"
#include "RenderTarget/RenderTarget.h"

#include <stdexcept>

RenderTarget::RenderTarget()
{
}

RenderTarget::~RenderTarget()
{
}

// セットアップ
void RenderTarget::Setup(ID3D12Device* device, const SetupParam& param)
{
	HRESULT hr;

	// resource
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	CD3DX12_RESOURCE_DESC resDesc(
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		static_cast<UINT>(param.width),
		static_cast<UINT>(param.height),
		1,
		1,
		format,
		1,
		0,
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
	);
	D3D12_CLEAR_VALUE clearValue;
	{
		FLOAT color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		memcpy(&clearValue.Color, color, sizeof(FLOAT) * 4);
		clearValue.Format = format;
	}

	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		&clearValue,
		IID_PPV_ARGS(m_resource.GetAddressOf())
	);
	if (FAILED(hr))
	{
		throw std::runtime_error("RenderTarget CreateCommittedResource failed");
	}
	// RTV
	D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		param.rtvHeap->GetCPUDescriptorHandleForHeapStart(),
		param.rtvOffset, 
		device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
	);
	D3D12_RENDER_TARGET_VIEW_DESC rtViewDesc = {};
	{
		rtViewDesc.Format = format;
		rtViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	}
	device->CreateRenderTargetView(m_resource.Get(), &rtViewDesc, rtvCpuHandle);
	// SRV
	UINT srvHeapIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	auto srvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		param.srvHeap->GetCPUDescriptorHandleForHeapStart(), 
		param.srvOffset, 
		srvHeapIncrementSize
	);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	{
		srvDesc.Texture2D.MipLevels = resDesc.MipLevels;
		srvDesc.Format = format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	}
	device->CreateShaderResourceView(m_resource.Get(), &srvDesc, srvHandle);
	m_view = CD3DX12_GPU_DESCRIPTOR_HANDLE(
		param.srvHeap->GetGPUDescriptorHandleForHeapStart(),
		param.srvOffset,
		srvHeapIncrementSize
	);
}

// リソースバリア設置
void RenderTarget::SetResourceBarrier(ID3D12GraphicsCommandList* commandList, ResourceBarrierType type)
{
	if (m_resourceBarrierType == type) { return; }

	auto curState = GetResourceState(m_resourceBarrierType);
	auto nextState = GetResourceState(type);

	// レンダーターゲット描画可能からピクセルシェーダーリソースへ
	auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_resource.Get(),
		curState,
		nextState
	);
	commandList->ResourceBarrier(1, &resourceBarrier);
}

// リソースステート取得
D3D12_RESOURCE_STATES RenderTarget::GetResourceState(ResourceBarrierType type) const
{
	switch (type)
	{
	case RenderTarget::ResourceBarrierType::RenderTarget:			return D3D12_RESOURCE_STATE_RENDER_TARGET;				break;
	case RenderTarget::ResourceBarrierType::NonPixelShaderResource:	return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;	break;
	case RenderTarget::ResourceBarrierType::PixelShaderResource:	return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;		break;
	default:
		throw std::runtime_error("RenderTarget::GetResourceState() invalid type.");
		break;
	}

	return D3D12_RESOURCE_STATE_RENDER_TARGET;
}
