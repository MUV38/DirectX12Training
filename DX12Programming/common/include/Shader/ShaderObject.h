#pragma once

#include "../framework.h"

//! @brief �V�F�[�_�[�I�u�W�F�N�g
class ShaderObject
{
public:
	ShaderObject();
	~ShaderObject();

private:
	Microsoft::WRL::ComPtr<ID3DBlob> mBlob;
};