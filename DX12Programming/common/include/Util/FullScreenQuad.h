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
		/// RenderTargetView
		UINT numRenderTargets;						///< レンダーターゲット数
		DXGI_FORMAT rtvFormats[8];					///< RTVフォーマット
		/// DepthStencilView
		DXGI_FORMAT dsvFormat;						///< DSVフォーマット
		/// シェーダー
		Microsoft::WRL::ComPtr<ID3DBlob> ps;		///< ピクセルシェーダー
		Microsoft::WRL::ComPtr<ID3DBlob> signature;	///< シグネチャ

		SetupParam()
			: numRenderTargets(1)
			, dsvFormat(DXGI_FORMAT_D32_FLOAT)
			, ps(nullptr)
			, signature(nullptr)
		{
			for (int i = 0; i < 8; ++i)
			{
				rtvFormats[i] = DXGI_FORMAT_UNKNOWN;
			}
			rtvFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		}
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
	/// ComPtr
	template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	
	/// 頂点
	struct Vertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 uv;
	};

	/// インデックス
	using Index = uint16_t;

private:
	ComPtr<ID3D12Resource> m_vb;
	ComPtr<ID3D12Resource> m_ib;

	D3D12_VERTEX_BUFFER_VIEW m_vbView;
	D3D12_INDEX_BUFFER_VIEW m_ibView;

	ComPtr<ID3DBlob> m_vs, m_ps;

	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pipeline;
};