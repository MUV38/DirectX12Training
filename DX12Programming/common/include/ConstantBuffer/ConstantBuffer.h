#pragma once

#include "../framework.h"
#include "Descriptor/DescriptorHandle.h"
#include "Descriptor/DescriptorManager.h"

//! @brief �R���X�^���g�o�b�t�@
class ConstantBuffer
{
public:
	ConstantBuffer();
	~ConstantBuffer();

	//! @brief �쐬
	void Create(ID3D12Device* device, DescriptorManager* descriptorManager, size_t bufferSize);
	
	//! @brief ���\�[�X
	Microsoft::WRL::ComPtr<ID3D12Resource>& getResource();

	//! @brief �f�X�N���v�^�[�n���h��
	const DescriptorHandle& getDescriptorHandle() const;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;	//!< ���\�[�X
	DescriptorHandle m_descriptorHandle;				//!< �f�X�N���v�^�[�n���h��
};