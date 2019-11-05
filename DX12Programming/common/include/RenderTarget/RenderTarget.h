#pragma once

#include "../framework.h"

/**
 * @brief レンダーターゲットクラス
 */
class RenderTarget
{
public:
	RenderTarget();
	~RenderTarget();

public:
	/**
	 * @brief セットアップパラメータ
	 */
	struct SetupParam
	{
		UINT width, height;
		DXGI_FORMAT format;

		ID3D12DescriptorHeap* rtvHeap;
		INT rtvOffset;

		ID3D12DescriptorHeap* srvHeap;
		INT srvOffset;
	};
	/**
	 * @brief セットアップ
	 * @param [in] device デバイス
	 * @param [in] param セットアップパラメータ
	 */
	void Setup(ID3D12Device* device, const SetupParam& param);

	/**
	 * @brief リソースバリアタイプ
	 */
	enum class ResourceBarrierType
	{
		RenderTarget,
		NonPixelShaderResource,
		PixelShaderResource,
	};
	/**
	 * @brief リソースバリア設置
	 * @param [in] commandList コマンドリスト
	 * @param [in] type リソースバリアタイプ
	 */
	void SetResourceBarrier(ID3D12GraphicsCommandList* commandList, ResourceBarrierType type);

private:
	/// リソースステート取得
	D3D12_RESOURCE_STATES GetResourceState(ResourceBarrierType type) const;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
	D3D12_GPU_DESCRIPTOR_HANDLE m_view;
	
	ResourceBarrierType m_resourceBarrierType;
};