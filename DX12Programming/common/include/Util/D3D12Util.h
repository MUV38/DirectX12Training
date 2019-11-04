#pragma once

#include "../framework.h"

namespace D3D12Util {

/**
 * @brief バッファ作成
 * @param [in] device デバイス
 * @param [in] bufferSize バッファサイズ
 * @param [in] initialData 初期化データ
 * @return リソース
 */
Microsoft::WRL::ComPtr<ID3D12Resource> CreateBuffer(ID3D12Device* device, size_t bufferSize, const void* initialData);

/**
 * @brief シェーダーコンパイル
 * @param [in] filePath ファイルパス
 * @param [in] targetProfile ターゲットプロファイル(L"ps_6_0"など)
 * @param [in] shaderBlob シェーダーBlob
 * @param [in] errorBlob エラーBlob
 * @return 成功か
 */
HRESULT CompileShaderFromFile(
	const std::wstring& filePath, 
	const std::wstring& targetProfile, 
	Microsoft::WRL::ComPtr<ID3DBlob>& shaderBlob, 
	Microsoft::WRL::ComPtr<ID3DBlob>& errorBlob
);

} // namespace D3D12Util