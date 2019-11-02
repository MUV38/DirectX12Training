#pragma once

#include "../framework.h"

/**
 * @brief �e�N�X�`���N���X
 */
class Texture
{
public:
	Texture();
	~Texture();

	/**
	 * @brief ���\�[�X�ݒ�
	 * @param [in] resource ���\�[�X
	 */
	void SetResource(ID3D12Resource* resource) { m_resource = resource; }
	/**
	 * @brief ���\�[�X�擾
	 */
	Microsoft::WRL::ComPtr<ID3D12Resource> GetResource() const { return m_resource; }
	
	/**
	 * @biref �V�F�[�_�[���\�[�X�r���[�ݒ�
	 * @param [in] srv �V�F�[�_�[���\�[�X�r���[
	 */
	void SetShaderResourceView(D3D12_GPU_DESCRIPTOR_HANDLE srv) { m_srv = srv; }
	/**
	 * @brief �V�F�[�_�[���\�[�X�r���[�擾
	 */
	D3D12_GPU_DESCRIPTOR_HANDLE GetShaderResourceView() const { return m_srv; }

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
	D3D12_GPU_DESCRIPTOR_HANDLE m_srv;
};