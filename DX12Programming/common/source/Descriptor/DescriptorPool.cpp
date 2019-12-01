#include "Descriptor/DescriptorPool.h"
#include "Util/Assert.h"

DescriptorPool::DescriptorPool()
	: m_heap()
	, m_numDescriptors(0)
	, m_descriptorHandleIncrementSize(0)
	, m_curDescriptorIndex(0)
{
}

DescriptorPool::~DescriptorPool()
{
	Release();
}

// ������
bool DescriptorPool::Init(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc)
{
	HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heap));
	if (FAILED(hr))
	{
		return false;
	}

	m_numDescriptors = desc.NumDescriptors;
	m_descriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(desc.Type);
	m_freeHandleList.reserve(desc.NumDescriptors);

	return true;
}

// ���
void DescriptorPool::Release()
{
	m_heap = nullptr;
	m_numDescriptors = 0;
	m_descriptorHandleIncrementSize = 0;
	m_curDescriptorIndex = 0;

	m_freeHandleList.clear();
	m_freeHandleList.shrink_to_fit();
}

// �f�X�N���v�^�[�m��
DescriptorHandle DescriptorPool::Alloc()
{
	// �󂫃��X�g����擾
	if (m_freeHandleList.size() > 0)
	{
		const auto& handle = m_freeHandleList.back();
		m_freeHandleList.pop_back();
		return handle;
	}

	m_curDescriptorIndex++;

	// �͈͊O�A�N�Z�X�`�F�b�N
	ASSERT(m_curDescriptorIndex < m_numDescriptors);

	return DescriptorHandle(m_heap.Get(), m_curDescriptorIndex, m_descriptorHandleIncrementSize);
}

// �f�X�N���v�^�[���
void DescriptorPool::Free(const DescriptorHandle descriptorHandle)
{
	// �q�[�v���Ⴄ�ꍇ�͖���
	if (descriptorHandle.GetHeap() == m_heap.Get()) { return; }

	// �󂫃��X�g�ɓo�^�ς݂̃n���h���͖���
	for (auto& handle : m_freeHandleList)
	{
		if (handle == descriptorHandle) { return; }
	}

	m_freeHandleList.push_back(descriptorHandle);
}