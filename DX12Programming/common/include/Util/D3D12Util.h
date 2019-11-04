#pragma once

#include "../framework.h"

namespace D3D12Util {

/**
 * @brief バッファ作成
 * @param [in] device デバイス
 * @param [in] bufferSize バッファサイズ
 * @param [in] initialData 初期化データ
 */
Microsoft::WRL::ComPtr<ID3D12Resource> CreateBuffer(ID3D12Device* device, size_t bufferSize, const void* initialData);

} // namespace D3D12Util