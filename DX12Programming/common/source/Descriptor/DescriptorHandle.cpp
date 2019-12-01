#include "Descriptor/DescriptorHandle.h"

#include "D3D12/d3dx12.h"

DescriptorHandle::DescriptorHandle()
	: m_heap(nullptr)
	, m_offsetInDescriptors(0)
	, m_cpuHandle()
	, m_gpuHandle()
{
}

DescriptorHandle::DescriptorHandle(ID3D12DescriptorHeap* heap, INT offsetInDescriptors, UINT descriptorIncrementSize)
{
	Init(heap, offsetInDescriptors, descriptorIncrementSize);
}

DescriptorHandle::~DescriptorHandle()
{
}

// ������
void DescriptorHandle::Init(ID3D12DescriptorHeap* heap, INT offsetInDescriptors, UINT descriptorIncrementSize)
{
	m_heap = heap;
	m_offsetInDescriptors = offsetInDescriptors;
	
	m_cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heap->GetCPUDescriptorHandleForHeapStart(), offsetInDescriptors, descriptorIncrementSize);
	m_gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(heap->GetGPUDescriptorHandleForHeapStart(), offsetInDescriptors, descriptorIncrementSize);
}

// ==�I�y���[�^�[�̃I�[�o�[���[�h
bool DescriptorHandle::operator==(const DescriptorHandle& rhs)
{
	return m_heap == rhs.m_heap && m_offsetInDescriptors == rhs.m_offsetInDescriptors;
}
