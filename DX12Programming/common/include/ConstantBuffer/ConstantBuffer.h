#pragma once

#include "../framework.h"
#include "Descriptor/DescriptorHandle.h"
#include "Descriptor/DescriptorManager.h"
#include "Util/FrameResource.h"

//! @brief �R���X�^���g�o�b�t�@
class ConstantBuffer
{
public:
	ConstantBuffer();
	~ConstantBuffer();

	//! @brief �쐬
	void create(ID3D12Device* device, DescriptorManager* descriptorManager, size_t bufferSize);
	
	//! @brief Map
	void map(uint32_t frameIndex, void** ptr);

	//! @brief Unmap
	void unmap(uint32_t frameIndex);
	
	//! @brief �r���[
	const D3D12_GPU_DESCRIPTOR_HANDLE& getView(uint32_t frameIndex) const;

private:
	static const uint32_t FrameBufferCount = FrameResource::FrameBufferCount;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> mResource[FrameBufferCount];	//!< ���\�[�X
	DescriptorHandle mDescriptorHandle[FrameBufferCount];					//!< �f�X�N���v�^�[�n���h��
};