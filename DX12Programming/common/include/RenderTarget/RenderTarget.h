#pragma once

#include "../framework.h"
#include "Descriptor/DescriptorPool.h"

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

		DescriptorPool* rtvDescriptorPool;
		DescriptorPool* cbvSrvUavDescriptorPool;

		bool allowUav;
		
		SetupParam()
			: width(1)
			, height(1)
			, format(DXGI_FORMAT_R8G8B8A8_UNORM)
			, rtvDescriptorPool(nullptr)
			, cbvSrvUavDescriptorPool(nullptr)
			, allowUav(false)
		{}
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

	/**
	 * @brief リソース取得
	 * @return リソース
	 */
	ID3D12Resource* GetResource() const { return m_resource.Get(); }

	/**
	 * @brief RTVデスクリプターハンドル取得
	 * @return RTVデスクリプターハンドル
	 */
	const DescriptorHandle& GetRtvDescriptorHandle() const { return m_rtv; }

	/**
	 * @brief ShaderResourceView取得
	 * @return ShaderResourceView
	 */
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetShaderResourceView() const { return m_srv.GetGPUHandle(); }

	/**
	 * @brief UnorderedAccessView取得
	 * @return UnorderedAccessView
	 */
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetUnorderedAccessView() const { return m_uav.GetGPUHandle(); }

private:
	/// リソースステート取得
	D3D12_RESOURCE_STATES GetResourceState(ResourceBarrierType type) const;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
	DescriptorHandle m_rtv;
	DescriptorHandle m_srv;
	DescriptorHandle m_uav;
	
	ResourceBarrierType m_resourceBarrierType;
};