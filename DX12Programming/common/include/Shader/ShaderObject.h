#pragma once

#include "../framework.h"

//! @brief �V�F�[�_�[�I�u�W�F�N�g
class ShaderObject
{
public:
	ShaderObject();
	~ShaderObject();
	
	//! @brief �R���p�C��
	void compile(const wchar_t* filePathStr, const wchar_t* targetProfileStr);

	//! @brief �V�F�[�_�[�o�C�g�R�[�h
	const CD3DX12_SHADER_BYTECODE& getShaderByteCode() const;

private:
	Microsoft::WRL::ComPtr<ID3DBlob> mBlob;
	CD3DX12_SHADER_BYTECODE mByteCode;
};