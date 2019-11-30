#pragma once

#include "../framework.h"
#include "Descriptor/DescriptorPool.h"
#include "Descriptor/DescriptorHandle.h"

/**
 * @brief �e�N�X�`���N���X
 */
class Texture
{
public:
	Texture();
	~Texture();

	/**
	 * @brief ������
	 */
	void Init(ID3D12Resource* resource, DescriptorPool* descriptorPool, DescriptorHandle descriptorHandle);

	/**
	 * @brief ���
	 */
	void Release();

	/**
	 * @brief ���\�[�X�擾
	 */
	Microsoft::WRL::ComPtr<ID3D12Resource> GetResource() const { return m_resource; }
	
	/**
	 * @brief �V�F�[�_�[���\�[�X�r���[�擾
	 */
	D3D12_GPU_DESCRIPTOR_HANDLE GetShaderResourceView() const { return m_descriptorHandle.GetGPUHandle(); }

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
	DescriptorPool* m_descriptorPool;
	DescriptorHandle m_descriptorHandle;
};