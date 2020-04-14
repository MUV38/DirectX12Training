#include "ConstantBuffer/ConstantBuffer.h"

#include "D3D12/D3D12Util.h"
#include "Util/Assert.h"

ConstantBuffer::ConstantBuffer()
	: m_resource()
	, m_descriptorHandle()
{
}

ConstantBuffer::~ConstantBuffer()
{
}

// çÏê¨
void ConstantBuffer::Create(ID3D12Device* device, DescriptorManager* descriptorManager, size_t bufferSize)
{
	if (!device) { return; }
	if (!descriptorManager) { return; }

	size_t alignedBufferSize = bufferSize + 255 & ~255;
	
	for (uint32_t i = 0; i < FrameBufferCount; ++i)
	{
		m_resource[i] = D3D12Util::CreateBuffer(device, alignedBufferSize, nullptr);

		m_descriptorHandle[i] = descriptorManager->Alloc(DescriptorManager::DescriptorPoolType::CbvSrvUav);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbDesc{};
		cbDesc.BufferLocation = m_resource[i]->GetGPUVirtualAddress();
		cbDesc.SizeInBytes = static_cast<UINT>(alignedBufferSize);
		device->CreateConstantBufferView(&cbDesc, m_descriptorHandle[i].GetCPUHandle());
	}
}

// Map
void ConstantBuffer::Map(uint32_t frameIndex, void** ptr)
{
	if (m_resource->Get())
	{
		D3D12_RANGE range{ 0, 0 };
		m_resource[frameIndex]->Map(0, &range, ptr);
	}
}

// Unmap
void ConstantBuffer::Unmap(uint32_t frameIndex)
{
	if (m_resource->Get())
	{
		m_resource[frameIndex]->Unmap(0, nullptr);
	}
}

const D3D12_GPU_DESCRIPTOR_HANDLE& ConstantBuffer::getView(uint32_t frameIndex) const
{
	ASSERT(frameIndex < FrameBufferCount);

	return m_descriptorHandle->GetGPUHandle();
}