#pragma once

#include "../framework.h"

namespace D3D12Util {

/**
 * @brief �o�b�t�@�쐬
 * @param [in] device �f�o�C�X
 * @param [in] bufferSize �o�b�t�@�T�C�Y
 * @param [in] initialData �������f�[�^
 * @return ���\�[�X
 */
Microsoft::WRL::ComPtr<ID3D12Resource> CreateBuffer(ID3D12Device* device, size_t bufferSize, const void* initialData);

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