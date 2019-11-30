#pragma once

#include <vector>
#include <wrl.h>
#include <d3d12.h>
#include "DescriptorHandle.h"

/**
 * @brief デスクリプタープール
 */
class DescriptorPool
{
public:
	DescriptorPool();
	~DescriptorPool();

	/**
	 * @brief 初期化
	 */
	bool Init(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc);

	/**
	 * @brief 解放
	 */
	void Release();

	/**
	 * @brief デスクリプター確保
	 */
	DescriptorHandle Alloc();

	/**
	 * @brief デスクリプター解放
	 */
	void Free(const DescriptorHandle descriptorHandle);

	/**
	 * @brief ヒープ取得
	 */
	ID3D12DescriptorHeap* GetHeap() const { return m_heap.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap;
	UINT m_numDescriptors;
	UINT m_descriptorHandleIncrementSize;

	UINT m_curDescriptorIndex;
	std::vector<DescriptorHandle> m_freeHandleList;
};