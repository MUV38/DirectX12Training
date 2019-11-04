#pragma once

#include "../framework.h"

namespace D3D12Util {

/**
 * @brief �o�b�t�@�쐬
 * @param [in] device �f�o�C�X
 * @param [in] bufferSize �o�b�t�@�T�C�Y
 * @param [in] initialData �������f�[�^
 */
Microsoft::WRL::ComPtr<ID3D12Resource> CreateBuffer(ID3D12Device* device, size_t bufferSize, const void* initialData);

} // namespace D3D12Util