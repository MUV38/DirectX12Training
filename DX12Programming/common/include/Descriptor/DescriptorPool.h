#pragma once

#include <vector>
#include <wrl.h>
#include <d3d12.h>
#include "DescriptorHandle.h"

/**
 * @brief �f�X�N���v�^�[�v�[��
 */
class DescriptorPool
{
public:
	DescriptorPool();
	~DescriptorPool();

	/**
	 * @brief ������
	 */
	bool Init(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc);

	/**
	 * @brief ���
	 */
	void Release();

	/**
	 * @brief �f�X�N���v�^�[�m��
	 */
	DescriptorHandle Alloc();

	/**
	 * @brief �f�X�N���v�^�[���
	 */
	void Free(const DescriptorHandle descriptorHandle);

	/**
	 * @brief �q�[�v�擾
	 */
	ID3D12DescriptorHeap* GetHeap() const { return m_heap.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap;
	UINT m_numDescriptors;
	UINT m_descriptorHandleIncrementSize;

	UINT m_curDescriptorIndex;
	std::vector<DescriptorHandle> m_freeHandleList;
};