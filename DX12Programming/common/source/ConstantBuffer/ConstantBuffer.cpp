#include "ConstantBuffer/ConstantBuffer.h"

#include "D3D12/D3D12Util.h"
#include "Util/Assert.h"

ConstantBuffer::ConstantBuffer()
	: mResource()
	, mDescriptorHandle()
{
}

ConstantBuffer::~ConstantBuffer()
{
}

// çÏê¨
void ConstantBuffer::create(ID3D12Device* device, DescriptorManager* descriptorManager, size_t bufferSize)
{
	if (!device) { return; }
	if (!descriptorManager) { return; }

	size_t alignedBufferSize = bufferSize + 255 & ~255;
	
	for (uint32_t i = 0; i < FrameBufferCount; ++i)
	{
		mResource[i] = D3D12Util::CreateBuffer(device, alignedBufferSize, nullptr);

		mDescriptorHandle[i] = descriptorManager->Alloc(DescriptorManager::DescriptorPoolType::CbvSrvUav);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbDesc{};
		cbDesc.BufferLocation = mResource[i]->GetGPUVirtualAddress();
		cbDesc.SizeInBytes = static_cast<UINT>(alignedBufferSize);
		device->CreateConstantBufferView(&cbDesc, mDescriptorHandle[i].GetCPUHandle());
	}
}

// Map
void ConstantBuffer::map(uint32_t frameIndex, void** ptr)
{
	if (mResource->Get())
	{
		D3D12_RANGE range{ 0, 0 };
		mResource[frameIndex]->Map(0, &range, ptr);
	}
}

// Unmap
void ConstantBuffer::unmap(uint32_t frameIndex)
{
	if (mResource->Get())
	{
		mResource[frameIndex]->Unmap(0, nullptr);
	}
}

const D3D12_GPU_DESCRIPTOR_HANDLE& ConstantBuffer::getView(uint32_t frameIndex) const
{
	ASSERT(frameIndex < FrameBufferCount);

	return mDescriptorHandle->GetGPUHandle();
}