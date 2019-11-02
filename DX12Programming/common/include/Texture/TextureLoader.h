#pragma once

#include "../framework.h"

class Texture;

/**
 * @brief テクスチャローダー
 */
class TextureLoader
{
public:
	/**
	 * @brief DDS読み込み
	 * @param [in] device デバイス
	 * @param [in] filePath ファイルパス
	 * @param [in] heap デスクリプターヒープ
	 * @param [in] offsetInDescriptors デスクリプターオフセット
	 * @param [in] descriptorIncrementSize デスクリプターのインクリメントサイズ
	 * @param [in] commandAllocator コマンドアロケーター
	 * @param [in] commandQueue コマンドキュー
	 * @param [out] texture 出力先テクスチャ
	 */
	static HRESULT LoadDDS(
		ID3D12Device* device, 
		const wchar_t* filePath, 
		ID3D12DescriptorHeap* heap, 
		INT offsetInDescriptors, 
		UINT descriptorIncrementSize, 
		ID3D12CommandAllocator* commandAlocator,
		ID3D12CommandQueue* commandQueue,
		Texture* texture
	);

private:

};