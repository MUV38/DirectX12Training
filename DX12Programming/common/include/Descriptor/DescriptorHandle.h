#pragma once

#include <d3d12.h>

/**
 * @brief デスクリプターハンドル
 */
class DescriptorHandle
{
public:
	DescriptorHandle();
	DescriptorHandle(ID3D12DescriptorHeap* heap, INT offsetInDescriptors, UINT descriptorIncrementSize);
	~DescriptorHandle();

	/**
	 * @brief 初期化
	 */
	void Init(ID3D12DescriptorHeap* heap, INT offsetInDescriptors, UINT descriptorIncrementSize);

	/**
	 * @brief CPUハンドル取得
	 */
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetCPUHandle() const { return m_cpuHandle; }
	/**
	 * @brief GPUハンドル取得
	 */
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetGPUHandle() const { return m_gpuHandle; }

	/**
	 * @brief ヒープ取得
	 */
	ID3D12DescriptorHeap* GetHeap() const { return m_heap; }

	/**
	 * @brief オフセット取得
	 */
	INT GetOffset() const { return m_offsetInDescriptors; }

public:
	/// ==オペレーターのオーバーロード
	bool operator==(const DescriptorHandle& rhs);

private:
	ID3D12DescriptorHeap* m_heap;
	INT m_offsetInDescriptors;

	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle;
};