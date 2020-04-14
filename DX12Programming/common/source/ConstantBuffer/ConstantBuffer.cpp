#include "ConstantBuffer/ConstantBuffer.h"
#include "D3D12/D3D12Util.h"

ConstantBuffer::ConstantBuffer()
	: m_resource()
	, m_descriptorHandle()
{
}

ConstantBuffer::~ConstantBuffer()
{
}

// 作成
void ConstantBuffer::Create(ID3D12Device* device, DescriptorManager* descriptorManager, size_t bufferSize)
{
	if (!device) { return; }
	if (!descriptorManager) { return; }

	size_t alignedBufferSize = bufferSize + 255 & ~255;
	
	m_resource = D3D12Util::CreateBuffer(device, alignedBufferSize, nullptr);

	m_descriptorHandle = descriptorManager->Alloc(DescriptorManager::DescriptorPoolType::CbvSrvUav);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbDesc{};
	cbDesc.BufferLocation = m_resource->GetGPUVirtualAddress();
	cbDesc.SizeInBytes = static_cast<UINT>(alignedBufferSize);
	device->CreateConstantBufferView(&cbDesc, m_descriptorHandle.GetCPUHandle());
}

// リソース
Microsoft::WRL::ComPtr<ID3D12Resource>& ConstantBuffer::getResource()
{
	return m_resource;
}

// デスクリプターハンドル
const DescriptorHandle& ConstantBuffer::getDescriptorHandle() const
{
	return m_descriptorHandle;
}
