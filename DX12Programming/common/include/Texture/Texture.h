#pragma once

#include "../framework.h"

/**
 * @brief テクスチャクラス
 */
class Texture
{
public:
	Texture();
	~Texture();

	/**
	 * @brief リソース設定
	 * @param [in] resource リソース
	 */
	void SetResource(ID3D12Resource* resource) { m_resource = resource; }
	/**
	 * @brief リソース取得
	 */
	Microsoft::WRL::ComPtr<ID3D12Resource> GetResource() const { return m_resource; }
	
	/**
	 * @biref シェーダーリソースビュー設定
	 * @param [in] srv シェーダーリソースビュー
	 */
	void SetShaderResourceView(D3D12_GPU_DESCRIPTOR_HANDLE srv) { m_srv = srv; }
	/**
	 * @brief シェーダーリソースビュー取得
	 */
	D3D12_GPU_DESCRIPTOR_HANDLE GetShaderResourceView() const { return m_srv; }

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
	D3D12_GPU_DESCRIPTOR_HANDLE m_srv;
};