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

		ID3D12DescriptorHeap* srvUavHeap;
		INT srvOffset;
		INT uavOffset;

		SetupParam()
			: width(1)
			, height(1)
			, format(DXGI_FORMAT_R8G8B8A8_UNORM)
			, rtvHeap(nullptr)
			, rtvOffset(-1)
			, srvUavHeap(nullptr)
			, srvOffset(-1)
			, uavOffset(-1)
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
	 * @brief ShaderResourceView取得
	 * @return ShaderResourceView
	 */
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetShaderResourceView() const { return m_srv; }

	/**
	 * @brief UnorderedAccessView取得
	 * @return UnorderedAccessView
	 */
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetUnorderedAccessView() const { return m_uav; }

private:
	/// リソースステート取得
	D3D12_RESOURCE_STATES GetResourceState(ResourceBarrierType type) const;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
	D3D12_GPU_DESCRIPTOR_HANDLE m_srv;
	D3D12_GPU_DESCRIPTOR_HANDLE m_uav;
	
	ResourceBarrierType m_resourceBarrierType;
};