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

// 初期化
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

// 解放
void DescriptorPool::Release()
{
	m_heap = nullptr;
	m_numDescriptors = 0;
	m_descriptorHandleIncrementSize = 0;
	m_curDescriptorIndex = 0;

	m_freeHandleList.clear();
	m_freeHandleList.shrink_to_fit();
}

// デスクリプター確保
DescriptorHandle DescriptorPool::Alloc()
{
	// 空きリストから取得
	if (m_freeHandleList.size() > 0)
	{
		const auto& handle = m_freeHandleList.back();
		m_freeHandleList.pop_back();
		return handle;
	}

	m_curDescriptorIndex++;

	// 範囲外アクセスチェック
	ASSERT(m_curDescriptorIndex < m_numDescriptors);

	return DescriptorHandle(m_heap.Get(), m_curDescriptorIndex, m_descriptorHandleIncrementSize);
}

// デスクリプター解放
void DescriptorPool::Free(const DescriptorHandle descriptorHandle)
{
	// ヒープが違う場合は無視
	if (descriptorHandle.GetHeap() == m_heap.Get()) { return; }

	// 空きリストに登録済みのハンドルは無視
	for (auto& handle : m_freeHandleList)
	{
		if (handle == descriptorHandle) { return; }
	}

	m_freeHandleList.push_back(descriptorHandle);
}