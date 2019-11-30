#pragma once

#include <d3d12.h>

/**
 * @brief �f�X�N���v�^�[�n���h��
 */
class DescriptorHandle
{
public:
	DescriptorHandle();
	DescriptorHandle(ID3D12DescriptorHeap* heap, INT offsetInDescriptors, UINT descriptorIncrementSize);
	~DescriptorHandle();

	/**
	 * @brief ������
	 */
	void Init(ID3D12DescriptorHeap* heap, INT offsetInDescriptors, UINT descriptorIncrementSize);

	/**
	 * @brief CPU�n���h���擾
	 */
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetCPUHandle() const { return m_cpuHandle; }
	/**
	 * @brief GPU�n���h���擾
	 */
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetGPUHandle() const { return m_gpuHandle; }

	/**
	 * @brief �q�[�v�擾
	 */
	ID3D12DescriptorHeap* GetHeap() const { return m_heap; }

	/**
	 * @brief �I�t�Z�b�g�擾
	 */
	INT GetOffset() const { return m_offsetInDescriptors; }

public:
	/// ==�I�y���[�^�[�̃I�[�o�[���[�h
	bool operator==(const DescriptorHandle& rhs);

private:
	ID3D12DescriptorHeap* m_heap;
	INT m_offsetInDescriptors;

	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle;
};