#pragma once

#include "../framework.h"

namespace D3D12Util {

/**
 * @brief �o�b�t�@�쐬
 * @param [in] device �f�o�C�X
 * @param [in] bufferSize �o�b�t�@�T�C�Y
 * @param [in] initialData �������f�[�^
 * @param [in] resourceName ���\�[�X��
 * @return ���\�[�X
 */
Microsoft::WRL::ComPtr<ID3D12Resource> CreateBuffer(
	ID3D12Device* device, 
	size_t bufferSize, 
	const void* initialData,
	const wchar_t* resourceName = nullptr
);

/**
 * @brief UAV�o�b�t�@�쐬
 * @param [in] device �f�o�C�X
 * @param [in] bufferSize �o�b�t�@�T�C�Y
 * @param [in] initialResourceState �������\�[�X�X�e�[�g
 * @param [in] resourceName ���\�[�X��
 * @return ���\�[�X
 */
Microsoft::WRL::ComPtr<ID3D12Resource> CreateUAVBuffer(
	ID3D12Device* device, 
	size_t bufferSize, 
	D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_COMMON,
	const wchar_t* resourceName = nullptr
);

/**
 * @brief �V�F�[�_�[�R���p�C��
 * @param [in] filePath �t�@�C���p�X
 * @param [in] targetProfile �^�[�Q�b�g�v���t�@�C��(L"ps_6_0"�Ȃ�)
 * @param [in] shaderBlob �V�F�[�_�[Blob
 * @param [in] errorBlob �G���[Blob
 * @return ������
 */
HRESULT CompileShaderFromFile(
	const std::wstring& filePath, 
	const std::wstring& targetProfile, 
	Microsoft::WRL::ComPtr<ID3DBlob>& shaderBlob, 
	Microsoft::WRL::ComPtr<ID3DBlob>& errorBlob
);

} // namespace D3D12Util