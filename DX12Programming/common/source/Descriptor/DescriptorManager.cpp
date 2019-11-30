#include "pch.h"
#include "Descriptor/DescriptorManager.h"

#include "Util/Assert.h"

DescriptorManager::DescriptorManager()
{
}

DescriptorManager::~DescriptorManager()
{
}

// ������
bool DescriptorManager::Init(ID3D12Device* device, UINT numCbvSrvUavPool, UINT numSamplerPool, UINT numRtvPool, UINT numDsvPool)
{
	// CBV_SRV_UAV
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc {
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			numCbvSrvUavPool,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
			0
		};
		if (!m_descriptorPool[static_cast<int>(DescriptorPoolType::CbvSrvUav)].Init(device, heapDesc))
		{
			ASSERT(false);
		}
	}
	// Sampler
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc {
			D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
			numSamplerPool,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
			0
		};
		if (!m_descriptorPool[static_cast<int>(DescriptorPoolType::Sampler)].Init(device, heapDesc))
		{
			ASSERT(false);
		}
	}
	// Rtv
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc{
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			numRtvPool,
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			0
		};
		if (!m_descriptorPool[static_cast<int>(DescriptorPoolType::Rtv)].Init(device, heapDesc))
		{
			ASSERT(false);
		}
	}
	// Dsv
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc{
			D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
			numDsvPool,
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			0
		};
		if (!m_descriptorPool[static_cast<int>(DescriptorPoolType::Dsv)].Init(device, heapDesc))
		{
			ASSERT(false);
		}
	}

	return true;
}

// ���
void DescriptorManager::Release()
{
	for (int i = 0; i < NumDescriptorPoolType; ++i)
	{
		m_descriptorPool[i].Release();
	}
}

// �f�X�N���v�^�[�m��
DescriptorHandle DescriptorManager::Alloc(DescriptorPoolType type)
{
	return m_descriptorPool[static_cast<int>(type)].Alloc();
}

// �f�X�N���v�^�[���
void DescriptorManager::Free(DescriptorPoolType type, const DescriptorHandle& handle)
{
	m_descriptorPool[static_cast<int>(type)].Free(handle);
}
