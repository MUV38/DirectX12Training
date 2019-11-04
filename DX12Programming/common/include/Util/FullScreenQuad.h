#pragma once

#include "../framework.h"

/**
 * @brief フルスクリーン描画
 */
class FullScreenQuad
{
public:
	FullScreenQuad();
	~FullScreenQuad();

public:
	/**
	 * @brief セットアップパラメータ
	 */
	struct SetupParam
	{

	};
	/**
	 * @brief セットアップ
	 * @param [in] param セットアップパラメータ
	 * 
	 * 描画するまでに１回必ず呼び出す
	 */
	void Setup(ID3D12Device* device, const SetupParam& param);

	/**
	 * @brief 描画
	 * @param [in] commandLiast コマンドリスト
	 */
	void Draw(ID3D12GraphicsCommandList* commandList);

private:
	/// 頂点
	struct Vertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 uv;
	};
	/// インデックス
	using Index = uint16_t;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vb;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_ib;

	D3D12_VERTEX_BUFFER_VIEW m_vbView;
	D3D12_INDEX_BUFFER_VIEW m_ibView;
};