#pragma once

#include "../framework.h"

class Texture;

/**
 * @brief �e�N�X�`�����[�_�[
 */
class TextureLoader
{
public:
	/**
	 * @brief DDS�ǂݍ���
	 * @param [in] device �f�o�C�X
	 * @param [in] filePath �t�@�C���p�X
	 * @param [in] heap �f�X�N���v�^�[�q�[�v
	 * @param [in] offsetInDescriptors �f�X�N���v�^�[�I�t�Z�b�g
	 * @param [in] descriptorIncrementSize �f�X�N���v�^�[�̃C���N�������g�T�C�Y
	 * @param [in] commandAllocator �R�}���h�A���P�[�^�[
	 * @param [in] commandQueue �R�}���h�L���[
	 * @param [out] texture �o�͐�e�N�X�`��
	 */
	static HRESULT LoadDDS(
		ID3D12Device* device, 
		const wchar_t* filePath, 
		ID3D12DescriptorHeap* heap, 
		INT offsetInDescriptors, 
		UINT descriptorIncrementSize, 
		ID3D12CommandAllocator* commandAlocator,
		ID3D12CommandQueue* commandQueue,
		Texture* texture
	);

private:

};