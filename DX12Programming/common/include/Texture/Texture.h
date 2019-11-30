#pragma once

#include "../framework.h"
#include "Descriptor/DescriptorPool.h"
#include "Descriptor/DescriptorHandle.h"

/**
 * @brief テクスチャクラス
 */
class Texture
{
public:
	Texture();
	~Texture();

	/**
	 * @brief 初期化
	 */
	void Init(ID3D12Resource* resource, DescriptorPool* descriptorPool, DescriptorHandle descriptorHandle);

	/**
	 * @brief 解放
	 */
	void Release();

	/**
	 * @brief リソース取得
	 */
	Microsoft::WRL::ComPtr<ID3D12Resource> GetResource() const { return m_resource; }
	
	/**
	 * @brief シェーダーリソースビュー取得
	 */
	D3D12_GPU_DESCRIPTOR_HANDLE GetShaderResourceView() const { return m_descriptorHandle.GetGPUHandle(); }

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
	DescriptorPool* m_descriptorPool;
	DescriptorHandle m_descriptorHandle;
};